#include "ScriptableObjectAnalyzer.h"
#include "TreeSitterEngine/CSharpParser.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <iostream>

namespace Unity {

ScriptableObjectAnalyzer::ScriptableObjectAnalyzer() {
    clear();
}

bool ScriptableObjectAnalyzer::analyzeFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_content = buffer.str();
    file.close();

    // Check if file contains ScriptableObject
    if (file_content.find("ScriptableObject") == std::string::npos) {
        return true; // Not an error, just not a ScriptableObject file
    }

    // Simple regex-based analysis for now (could be replaced with TreeSitter later)
    std::regex class_regex(R"(class\s+(\w+)\s*:\s*\w*ScriptableObject)");
    std::smatch matches;
    std::string::const_iterator searchStart(file_content.cbegin());

    while (std::regex_search(searchStart, file_content.cend(), matches, class_regex)) {
        ScriptableObjectInfo so_info;
        so_info.class_name = matches[1].str();
        so_info.file_path = file_path;
        so_info.base_class = "ScriptableObject";
        
        // Find line numbers (simplified)
        std::string match_str = matches[0].str();
        size_t match_pos = matches.prefix().length();
        size_t lines_before = std::count(file_content.begin(), file_content.begin() + match_pos, '\n');
        so_info.start_line = static_cast<int>(lines_before + 1);
        so_info.end_line = so_info.start_line + 50; // Simplified - assume 50 lines
        so_info.total_lines = so_info.end_line - so_info.start_line + 1;

        // Parse CreateAssetMenu attribute
        parseCreateAssetMenuAttribute(file_content, so_info);
        
        // Parse serialized fields
        parseSerializedFieldsFromContent(file_content, so_info);
        
        // Extract methods (simplified)
        extractMethodsFromContent(file_content, so_info);
        
        // Detect asset references
        detectAssetReferences(so_info, file_content);
        
        m_scriptable_objects.push_back(so_info);
        
        searchStart = matches.suffix().first;
    }

    return true;
}

bool ScriptableObjectAnalyzer::analyzeProject(const std::vector<std::string>& file_paths) {
    clear();
    
    for (const auto& file_path : file_paths) {
        if (!analyzeFile(file_path)) {
            std::cerr << "Failed to analyze ScriptableObject file: " << file_path << std::endl;
            // Continue with other files
        }
    }
    
    return true;
}

void ScriptableObjectAnalyzer::parseCreateAssetMenuAttribute(const std::string& file_content, ScriptableObjectInfo& so_info) {
    // Look for [CreateAssetMenu] attribute before class declaration
    std::regex create_menu_regex(R"(\[CreateAssetMenu\s*\(([^)]*)\)\])");
    std::smatch matches;
    
    if (std::regex_search(file_content, matches, create_menu_regex)) {
        std::string params = matches[1].str();
        
        so_info.create_menu.is_valid = true;
        
        // Parse fileName parameter
        std::regex filename_regex(R"(fileName\s*=\s*\"([^\"]*)\")");
        std::smatch filename_match;
        if (std::regex_search(params, filename_match, filename_regex)) {
            so_info.create_menu.fileName = filename_match[1].str();
        }
        
        // Parse menuName parameter
        std::regex menuname_regex(R"(menuName\s*=\s*\"([^\"]*)\")");
        std::smatch menuname_match;
        if (std::regex_search(params, menuname_match, menuname_regex)) {
            so_info.create_menu.menuName = menuname_match[1].str();
        }
        
        // Parse order parameter (optional)
        std::regex order_regex(R"(order\s*=\s*(\d+))");
        std::smatch order_match;
        if (std::regex_search(params, order_match, order_regex)) {
            so_info.create_menu.order = std::stoi(order_match[1].str());
        } else {
            so_info.create_menu.order = 0;
        }
    }
}

void ScriptableObjectAnalyzer::parseSerializedFieldsFromContent(const std::string& file_content, ScriptableObjectInfo& so_info) {
    // Look for field declarations
    std::regex field_regex(R"((public|private|protected|\[SerializeField\])\s+[\w\[\]<>,\s]+\s+(\w+)\s*[;=])");
    std::smatch matches;
    std::string::const_iterator searchStart(file_content.cbegin());
    
    while (std::regex_search(searchStart, file_content.cend(), matches, field_regex)) {
        SerializedField field;
        std::string full_match = matches[0].str();
        field.name = matches[2].str();
        
        // Extract type (simplified)
        std::regex type_regex(R"((public|private|protected|\[SerializeField\])\s+([\w\[\]<>,\s]+)\s+\w+)");
        std::smatch type_match;
        if (std::regex_search(full_match, type_match, type_regex)) {
            field.type = type_match[2].str();
            // Clean up type string
            field.type.erase(std::remove_if(field.type.begin(), field.type.end(), ::isspace), field.type.end());
        }
        
        field.access_modifier = matches[1].str().find("SerializeField") != std::string::npos ? "private" : matches[1].str();
        field.is_array = field.type.find("[]") != std::string::npos;
        field.is_list = field.type.find("List<") != std::string::npos;
        
        // Check for attributes
        std::string before_field = matches.prefix().str();
        if (before_field.find("[Header(") != std::string::npos) {
            field.has_header = true;
            std::regex header_content_regex(R"(\[Header\s*\(\s*\"([^\"]*)\"\s*\)\])");
            std::smatch header_match;
            if (std::regex_search(before_field, header_match, header_content_regex)) {
                field.header_text = header_match[1].str();
            }
        }
        
        if (before_field.find("[Range(") != std::string::npos) {
            field.has_range = true;
            std::regex range_regex(R"(\[Range\s*\(\s*([\d.]+)\s*,\s*([\d.]+)\s*\)\])");
            std::smatch range_match;
            if (std::regex_search(before_field, range_match, range_regex)) {
                field.range_min = std::stof(range_match[1].str());
                field.range_max = std::stof(range_match[2].str());
            }
        }
        
        // Calculate line number (simplified)
        size_t match_pos = matches.prefix().length();
        size_t lines_before = std::count(file_content.begin(), file_content.begin() + match_pos, '\n');
        field.line_number = static_cast<int>(lines_before + 1);
        
        // Check if it's a Unity asset type
        if (isUnityAssetType(field.type)) {
            so_info.prefab_references.push_back(field.name);
            addAssetDependency(so_info.class_name, field.type, field.name, 
                             field.is_array ? "array" : "direct", field.line_number);
        }
        
        so_info.serialized_fields.push_back(field);
        searchStart = matches.suffix().first;
    }
}

void ScriptableObjectAnalyzer::extractMethodsFromContent(const std::string& file_content, ScriptableObjectInfo& so_info) {
    // Simple method extraction
    std::regex method_regex(R"((public|private|protected)\s+([\w<>\[\]]+)\s+(\w+)\s*\([^)]*\))");
    std::smatch matches;
    std::string::const_iterator searchStart(file_content.cbegin());
    
    while (std::regex_search(searchStart, file_content.cend(), matches, method_regex)) {
        ScriptableObjectMethod method;
        method.access_modifier = matches[1].str();
        method.return_type = matches[2].str();
        method.name = matches[3].str();
        
        // Calculate line number
        size_t match_pos = matches.prefix().length();
        size_t lines_before = std::count(file_content.begin(), file_content.begin() + match_pos, '\n');
        method.start_line = static_cast<int>(lines_before + 1);
        method.end_line = method.start_line; // Simplified
        
        method.is_unity_callback = (method.name == "OnValidate" || 
                                   method.name == "OnEnable" || 
                                   method.name == "OnDisable");
        
        so_info.methods.push_back(method);
        
        // Categorize methods
        if (isValidationMethod(method.name)) {
            so_info.validation_methods.push_back(method.name);
        }
        if (isLookupMethod(method.name)) {
            so_info.lookup_methods.push_back(method.name);
        }
        
        searchStart = matches.suffix().first;
    }
}

void ScriptableObjectAnalyzer::detectAssetReferences(ScriptableObjectInfo& so_info, const std::string& file_content) {
    // Look for Unity asset types in the content
    std::vector<std::string> unity_types = {
        "GameObject", "AudioClip", "Sprite", "Texture2D", "Material", 
        "Mesh", "Animation", "AnimationClip", "RuntimeAnimatorController",
        "ParticleSystem", "Font", "Shader"
    };
    
    for (const auto& unity_type : unity_types) {
        if (file_content.find(unity_type) != std::string::npos) {
            so_info.referenced_types.push_back(unity_type);
        }
    }
}

bool ScriptableObjectAnalyzer::isUnityAssetType(const std::string& type) const {
    static const std::vector<std::string> asset_types = {
        "GameObject", "AudioClip", "Sprite", "Texture2D", "Material", 
        "Mesh", "Animation", "AnimationClip", "RuntimeAnimatorController",
        "ParticleSystem", "Font", "Shader"
    };
    
    // Check for exact match or array/list types
    for (const auto& asset_type : asset_types) {
        if (type == asset_type || 
            type == asset_type + "[]" ||
            type.find("List<" + asset_type + ">") != std::string::npos) {
            return true;
        }
    }
    return false;
}

void ScriptableObjectAnalyzer::addAssetDependency(const std::string& source_class, 
                                                  const std::string& target_type,
                                                  const std::string& field_name, 
                                                  const std::string& dep_type, 
                                                  int line) {
    AssetDependency dep;
    dep.source_scriptable_object = source_class;
    dep.target_asset_type = target_type;
    dep.field_name = field_name;
    dep.dependency_type = dep_type;
    dep.line_number = line;
    
    m_asset_dependencies.push_back(dep);
}

bool ScriptableObjectAnalyzer::isValidationMethod(const std::string& method_name) const {
    return method_name == "OnValidate" || 
           method_name.find("Validate") != std::string::npos ||
           method_name.find("IsValid") != std::string::npos;
}

bool ScriptableObjectAnalyzer::isLookupMethod(const std::string& method_name) const {
    return method_name.find("Get") == 0 || 
           method_name.find("Find") == 0 ||
           method_name.find("Lookup") != std::string::npos;
}

std::vector<ScriptableObjectInfo> ScriptableObjectAnalyzer::getScriptableObjectsByType(const std::string& type) const {
    std::vector<ScriptableObjectInfo> results;
    for (const auto& so : m_scriptable_objects) {
        if (so.class_name.find(type) != std::string::npos) {
            results.push_back(so);
        }
    }
    return results;
}

std::vector<AssetDependency> ScriptableObjectAnalyzer::getDependenciesForScriptableObject(const std::string& class_name) const {
    std::vector<AssetDependency> results;
    for (const auto& dep : m_asset_dependencies) {
        if (dep.source_scriptable_object == class_name) {
            results.push_back(dep);
        }
    }
    return results;
}

std::unordered_map<std::string, int> ScriptableObjectAnalyzer::getAssetReferenceCount() const {
    std::unordered_map<std::string, int> counts;
    for (const auto& dep : m_asset_dependencies) {
        counts[dep.target_asset_type]++;
    }
    return counts;
}

bool ScriptableObjectAnalyzer::hasCreateAssetMenu(const std::string& class_name) const {
    for (const auto& so : m_scriptable_objects) {
        if (so.class_name == class_name) {
            return so.create_menu.is_valid;
        }
    }
    return false;
}

std::vector<std::string> ScriptableObjectAnalyzer::getConfigurationScriptableObjects() const {
    std::vector<std::string> configs;
    for (const auto& so : m_scriptable_objects) {
        if (so.class_name.find("Settings") != std::string::npos ||
            so.class_name.find("Config") != std::string::npos ||
            so.class_name.find("Configuration") != std::string::npos) {
            configs.push_back(so.class_name);
        }
    }
    return configs;
}

std::vector<std::string> ScriptableObjectAnalyzer::getDatabaseScriptableObjects() const {
    std::vector<std::string> databases;
    for (const auto& so : m_scriptable_objects) {
        if (so.class_name.find("Database") != std::string::npos ||
            so.class_name.find("Collection") != std::string::npos ||
            so.lookup_methods.size() > 2) { // Has multiple lookup methods
            databases.push_back(so.class_name);
        }
    }
    return databases;
}

void ScriptableObjectAnalyzer::clear() {
    m_scriptable_objects.clear();
    m_asset_dependencies.clear();
}

} // namespace Unity