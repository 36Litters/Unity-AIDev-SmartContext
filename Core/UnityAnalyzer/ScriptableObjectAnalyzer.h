#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Unity {

struct SerializedField {
    std::string name;
    std::string type;
    std::string access_modifier;
    bool is_array;
    bool is_list;
    bool has_header;
    std::string header_text;
    bool has_range;
    float range_min;
    float range_max;
    bool has_create_asset_menu;
    int line_number;
};

struct ScriptableObjectMethod {
    std::string name;
    std::string return_type;
    std::vector<std::string> parameters;
    std::string access_modifier;
    bool is_virtual;
    bool is_override;
    bool is_unity_callback;
    int start_line;
    int end_line;
};

struct CreateAssetMenuAttribute {
    std::string fileName;
    std::string menuName;
    int order;
    bool is_valid;
};

struct ScriptableObjectInfo {
    std::string class_name;
    std::string file_path;
    std::string base_class;
    
    CreateAssetMenuAttribute create_menu;
    std::vector<SerializedField> serialized_fields;
    std::vector<ScriptableObjectMethod> methods;
    
    // Asset reference tracking
    std::vector<std::string> referenced_types;
    std::vector<std::string> prefab_references;
    std::vector<std::string> audio_references;
    std::vector<std::string> material_references;
    
    // Validation and utility methods
    std::vector<std::string> validation_methods;
    std::vector<std::string> lookup_methods;
    
    int start_line;
    int end_line;
    int total_lines;
};

struct AssetDependency {
    std::string source_scriptable_object;
    std::string target_asset_type;
    std::string field_name;
    std::string dependency_type; // "direct", "array", "list"
    int line_number;
};

class ScriptableObjectAnalyzer {
public:
    ScriptableObjectAnalyzer();
    ~ScriptableObjectAnalyzer() = default;

    bool analyzeFile(const std::string& file_path);
    bool analyzeProject(const std::vector<std::string>& file_paths);
    
    const std::vector<ScriptableObjectInfo>& getScriptableObjects() const { return m_scriptable_objects; }
    const std::vector<AssetDependency>& getAssetDependencies() const { return m_asset_dependencies; }
    
    // Query methods
    std::vector<ScriptableObjectInfo> getScriptableObjectsByType(const std::string& type) const;
    std::vector<AssetDependency> getDependenciesForScriptableObject(const std::string& class_name) const;
    std::unordered_map<std::string, int> getAssetReferenceCount() const;
    
    // Analysis methods
    bool hasCreateAssetMenu(const std::string& class_name) const;
    std::vector<std::string> getConfigurationScriptableObjects() const;
    std::vector<std::string> getDatabaseScriptableObjects() const;
    
    void clear();

private:
    std::vector<ScriptableObjectInfo> m_scriptable_objects;
    std::vector<AssetDependency> m_asset_dependencies;
    
    // Helper methods
    bool isScriptableObjectClass(const std::string& class_declaration) const;
    void parseCreateAssetMenuAttribute(const std::string& file_content, ScriptableObjectInfo& so_info);
    void parseSerializedFieldsFromContent(const std::string& file_content, ScriptableObjectInfo& so_info);
    void extractMethodsFromContent(const std::string& file_content, ScriptableObjectInfo& so_info);
    bool isUnityAssetType(const std::string& type) const;
    
    // Asset reference detection
    void detectAssetReferences(ScriptableObjectInfo& so_info, const std::string& file_content);
    void addAssetDependency(const std::string& source_class, const std::string& target_type, 
                           const std::string& field_name, const std::string& dep_type, int line);
    
    // Method analysis
    bool isValidationMethod(const std::string& method_name) const;
    bool isLookupMethod(const std::string& method_name) const;
};

} // namespace Unity