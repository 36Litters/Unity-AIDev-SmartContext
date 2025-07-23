#include "UnityInterface.h"
#include "CoreEngine.h"
#include "ContextSummarizer/ProjectSummarizer.h"
#include <nlohmann/json.hpp>
#include <cstring>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>

using namespace UnityContextGen;
using json = nlohmann::json;

// Global state for error handling
static std::mutex g_errorMutex;
static std::string g_lastError;
static ProgressCallback g_progressCallback = nullptr;

// Helper functions for error handling
void SetLastError(const std::string& error) {
    std::lock_guard<std::mutex> lock(g_errorMutex);
    g_lastError = error;
}

std::string GetLastErrorInternal() {
    std::lock_guard<std::mutex> lock(g_errorMutex);
    return g_lastError;
}

// Helper function to allocate and return C-style string
const char* AllocateString(const std::string& str) {
    if (str.empty()) {
        return nullptr;
    }
    
    char* result = new char[str.length() + 1];
    strcpy(result, str.c_str());
    return result;
}

// Helper function to report progress
void ReportProgress(const std::string& message, float progress) {
    if (g_progressCallback) {
        g_progressCallback(message.c_str(), progress);
    }
}

// Convert C# options JSON to native AnalysisOptions
AnalysisOptions ParseOptionsFromJson(const char* optionsJson) {
    AnalysisOptions options;
    
    try {
        if (optionsJson && strlen(optionsJson) > 0) {
            json j = json::parse(optionsJson);
            
            // Parse analysis flags
            options.analyze_dependencies = j.value("analyzeDependencies", true);
            options.analyze_lifecycle = j.value("analyzeLifecycleMethods", true);
            options.detect_patterns = j.value("detectDesignPatterns", true);
            options.generate_metadata = j.value("generateMetadata", true);
            options.generate_context = j.value("generateContextSummary", true);
            
            // Parse output flags
            options.export_json = j.value("exportJson", true);
            options.verbose_output = j.value("verboseLogging", false);
            
            // Set output directory
            options.output_directory = j.value("outputDirectory", "./unity_context_analysis");
        }
    } catch (const std::exception& e) {
        SetLastError("Failed to parse options JSON: " + std::string(e.what()));
    }
    
    return options;
}

// Convert analysis result to JSON for Unity
std::string ResultToJson(const AnalysisResult& result) {
    try {
        json j;
        
        // Status
        j["Success"] = result.success;
        j["ErrorMessage"] = result.error_message;
        j["AnalysisDurationMs"] = result.analysis_duration_ms;
        
        // Basic project info from project_metadata
        j["ProjectType"] = "Unity Project";
        j["Architecture"] = "Component-based";
        j["QualityScore"] = 85; // Default value
        
        // Component counts
        j["MonoBehaviourCount"] = result.monobehaviours.size();
        j["ScriptableObjectCount"] = result.scriptable_objects.size();
        j["SystemGroupCount"] = 1;
        j["DependencyCount"] = result.dependency_graph.edges.size();
        j["AssetDependencyCount"] = result.asset_dependencies.size();
        j["DetectedPatternCount"] = result.patterns.size();
        
        // Components array
        j["Components"] = json::array();
        for (const auto& comp : result.monobehaviours) {
            json compJson;
            compJson["Name"] = comp.class_name;
            compJson["FilePath"] = comp.file_path;
            compJson["BaseClass"] = "MonoBehaviour"; // Default base class
            compJson["Purpose"] = "Unity Component"; // Default purpose
            compJson["UnityMethods"] = comp.unity_methods;
            compJson["CustomMethods"] = comp.custom_methods;
            compJson["SerializedFields"] = comp.serialized_fields;
            compJson["Dependencies"] = json::array(); // Empty array for now
            compJson["ComplexityScore"] = 75; // Default complexity
            compJson["StartLine"] = comp.start_line;
            compJson["EndLine"] = comp.end_line;
            
            j["Components"].push_back(compJson);
        }
        
        // Dependencies array
        j["Dependencies"] = json::array();
        for (const auto& dep : result.dependency_graph.edges) {
            json depJson;
            depJson["SourceComponent"] = dep.source_component;
            depJson["TargetComponent"] = dep.target_component;
            depJson["DependencyType"] = dep.dependency_type;
            depJson["Context"] = dep.method_context;
            depJson["LineNumber"] = static_cast<int>(dep.line_number);
            
            j["Dependencies"].push_back(depJson);
        }
        
        // ScriptableObjects array
        j["ScriptableObjects"] = json::array();
        for (const auto& so : result.scriptable_objects) {
            json soJson;
            soJson["Name"] = so.class_name;
            soJson["FilePath"] = so.file_path;
            soJson["BaseClass"] = so.base_class;
            soJson["StartLine"] = so.start_line;
            soJson["EndLine"] = so.end_line;
            soJson["TotalLines"] = so.total_lines;
            
            // CreateAssetMenu info
            soJson["HasCreateAssetMenu"] = so.create_menu.is_valid;
            if (so.create_menu.is_valid) {
                soJson["CreateAssetMenu"] = json::object();
                soJson["CreateAssetMenu"]["FileName"] = so.create_menu.fileName;
                soJson["CreateAssetMenu"]["MenuName"] = so.create_menu.menuName;
                soJson["CreateAssetMenu"]["Order"] = so.create_menu.order;
            }
            
            // Serialized fields
            soJson["SerializedFields"] = json::array();
            for (const auto& field : so.serialized_fields) {
                json fieldJson;
                fieldJson["Name"] = field.name;
                fieldJson["Type"] = field.type;
                fieldJson["AccessModifier"] = field.access_modifier;
                fieldJson["IsArray"] = field.is_array;
                fieldJson["IsList"] = field.is_list;
                fieldJson["HasHeader"] = field.has_header;
                if (field.has_header) {
                    fieldJson["HeaderText"] = field.header_text;
                }
                fieldJson["HasRange"] = field.has_range;
                if (field.has_range) {
                    fieldJson["RangeMin"] = field.range_min;
                    fieldJson["RangeMax"] = field.range_max;
                }
                fieldJson["LineNumber"] = field.line_number;
                
                soJson["SerializedFields"].push_back(fieldJson);
            }
            
            // Methods
            soJson["Methods"] = json::array();
            for (const auto& method : so.methods) {
                json methodJson;
                methodJson["Name"] = method.name;
                methodJson["ReturnType"] = method.return_type;
                methodJson["AccessModifier"] = method.access_modifier;
                methodJson["IsUnityCallback"] = method.is_unity_callback;
                methodJson["StartLine"] = method.start_line;
                methodJson["EndLine"] = method.end_line;
                
                soJson["Methods"].push_back(methodJson);
            }
            
            // Referenced types and categories
            soJson["ReferencedTypes"] = so.referenced_types;
            soJson["ValidationMethods"] = so.validation_methods;
            soJson["LookupMethods"] = so.lookup_methods;
            
            j["ScriptableObjects"].push_back(soJson);
        }
        
        // Asset Dependencies array
        j["AssetDependencies"] = json::array();
        for (const auto& dep : result.asset_dependencies) {
            json depJson;
            depJson["SourceScriptableObject"] = dep.source_scriptable_object;
            depJson["TargetAssetType"] = dep.target_asset_type;
            depJson["FieldName"] = dep.field_name;
            depJson["DependencyType"] = dep.dependency_type;
            depJson["LineNumber"] = dep.line_number;
            
            j["AssetDependencies"].push_back(depJson);
        }
        
        // Detected patterns
        j["DetectedPatterns"] = json::array();
        for (const auto& pattern : result.patterns) {
            json patternJson;
            patternJson["PatternName"] = pattern.pattern_name;
            patternJson["Description"] = pattern.description;
            patternJson["InvolvedComponents"] = pattern.involved_components;
            patternJson["ConfidenceScore"] = 0.8f; // Default confidence
            patternJson["Evidence"] = pattern.evidence;
            
            j["DetectedPatterns"].push_back(patternJson);
        }
        
        // Context information - using default values for now
        j["ProjectContext"] = "Unity Project";
        j["DevelopmentGuidelines"] = "Use Unity lifecycle methods appropriately";
        j["ArchitectureOverview"] = "Component-based architecture using MonoBehaviour";
        j["LLMPrompt"] = "# Unity Project Context\n\nThis is a Unity project with MonoBehaviour components.";
        
        return j.dump(2);
        
    } catch (const std::exception& e) {
        SetLastError("Failed to convert result to JSON: " + std::string(e.what()));
        return "{}";
    }
}

extern "C" {

UNITY_EXPORT const char* UNITY_CALL AnalyzeProject(const char* projectPath, const char* optionsJson) {
    try {
        if (!projectPath) {
            SetLastError("Project path cannot be null");
            return nullptr;
        }
        
        ReportProgress("Initializing analysis...", 0.0f);
        
        CoreEngine engine;
        AnalysisOptions options = ParseOptionsFromJson(optionsJson);
        
        // Verbose option removed from this interface
        
        ReportProgress("Analyzing project directory...", 0.1f);
        
        AnalysisResult result = engine.analyzeDirectory(projectPath, options);
        
        if (!result.success) {
            SetLastError(result.error_message);
            return nullptr;
        }
        
        ReportProgress("Generating context summary...", 0.9f);
        
        // Generate additional context using ProjectSummarizer
        if (options.generate_context) {
            try {
                Context::ProjectSummarizer summarizer;
                auto context_result = summarizer.generateProjectContext(result.project_metadata);
                result.project_context = context_result;
            } catch (const std::exception& e) {
                // Context generation failed, but don't fail the entire analysis
                SetLastError("Context generation failed: " + std::string(e.what()));
            }
        }
        
        ReportProgress("Analysis complete!", 1.0f);
        
        std::string jsonResult = ResultToJson(result);
        return AllocateString(jsonResult);
        
    } catch (const std::exception& e) {
        SetLastError("Analysis failed: " + std::string(e.what()));
        return nullptr;
    }
}

UNITY_EXPORT const char* UNITY_CALL AnalyzeFile(const char* filePath, const char* optionsJson) {
    try {
        if (!filePath) {
            SetLastError("File path cannot be null");
            return nullptr;
        }
        
        CoreEngine engine;
        AnalysisOptions options = ParseOptionsFromJson(optionsJson);
        
        // Verbose option removed from this interface
        
        AnalysisResult result = engine.analyzeSingleFile(filePath, options);
        
        if (!result.success) {
            SetLastError(result.error_message);
            return nullptr;
        }
        
        std::string jsonResult = ResultToJson(result);
        return AllocateString(jsonResult);
        
    } catch (const std::exception& e) {
        SetLastError("File analysis failed: " + std::string(e.what()));
        return nullptr;
    }
}

UNITY_EXPORT const char* UNITY_CALL GenerateLLMPrompt(const char* analysisResultJson) {
    try {
        if (!analysisResultJson) {
            SetLastError("Analysis result JSON cannot be null");
            return nullptr;
        }
        
        // Parse the analysis result JSON
        json j = json::parse(analysisResultJson);
        
        // Extract key information for prompt generation
        std::string projectType = j.value("ProjectType", "Unity Project");
        std::string architecture = j.value("Architecture", "Component-based");
        int monoBehaviourCount = j.value("MonoBehaviourCount", 0);
        int dependencyCount = j.value("DependencyCount", 0);
        
        // Build LLM prompt
        std::stringstream prompt;
        prompt << "# Unity Project Context\n\n";
        prompt << "**Type:** " << projectType << " | ";
        prompt << "**Architecture:** " << architecture << " | ";
        prompt << "**Components:** " << monoBehaviourCount << "\n\n";
        
        prompt << "## Component Overview\n";
        
        if (j.contains("Components") && j["Components"].is_array()) {
            for (const auto& comp : j["Components"]) {
                std::string name = comp.value("Name", "Unknown");
                std::string purpose = comp.value("Purpose", "No description available");
                std::string baseClass = comp.value("BaseClass", "");
                
                prompt << "- **" << name << "**";
                if (!baseClass.empty()) {
                    prompt << " (extends " << baseClass << ")";
                }
                prompt << ": " << purpose << "\n";
            }
        }
        
        prompt << "\n## Key Dependencies\n";
        if (j.contains("Dependencies") && j["Dependencies"].is_array() && dependencyCount > 0) {
            for (const auto& dep : j["Dependencies"]) {
                std::string source = dep.value("SourceComponent", "Unknown");
                std::string target = dep.value("TargetComponent", "Unknown");
                std::string type = dep.value("DependencyType", "Reference");
                
                prompt << "- " << source << " → " << target << " (" << type << ")\n";
            }
        } else {
            prompt << "- No significant dependencies detected\n";
        }
        
        prompt << "\n## Design Patterns\n";
        if (j.contains("DetectedPatterns") && j["DetectedPatterns"].is_array()) {
            bool hasPatterns = false;
            for (const auto& pattern : j["DetectedPatterns"]) {
                std::string name = pattern.value("PatternName", "Unknown");
                std::string description = pattern.value("Description", "");
                float confidence = pattern.value("ConfidenceScore", 0.0f);
                
                if (confidence > 0.5f) {
                    prompt << "- **" << name << "**: " << description << "\n";
                    hasPatterns = true;
                }
            }
            if (!hasPatterns) {
                prompt << "- Standard Unity component architecture\n";
            }
        }
        
        prompt << "\n## Development Guidelines\n";
        if (j.contains("DevelopmentGuidelines") && !j["DevelopmentGuidelines"].get<std::string>().empty()) {
            prompt << j["DevelopmentGuidelines"].get<std::string>() << "\n";
        } else {
            prompt << "- Use Unity lifecycle methods appropriately (Awake for initialization, Start for setup, Update for frame logic)\n";
            prompt << "- Cache component references in Awake() to avoid repeated GetComponent calls\n";
            prompt << "- Follow established patterns and naming conventions\n";
        }
        
        return AllocateString(prompt.str());
        
    } catch (const std::exception& e) {
        SetLastError("Prompt generation failed: " + std::string(e.what()));
        return nullptr;
    }
}

UNITY_EXPORT const char* UNITY_CALL GenerateProjectSummary(const char* analysisResultJson) {
    try {
        if (!analysisResultJson) {
            SetLastError("Analysis result JSON cannot be null");
            return nullptr;
        }
        
        json j = json::parse(analysisResultJson);
        
        std::stringstream summary;
        summary << "Unity Project Analysis Summary\n";
        summary << "==============================\n\n";
        
        summary << "Project Type: " << j.value("ProjectType", "Unknown") << "\n";
        summary << "Architecture: " << j.value("Architecture", "Unknown") << "\n";
        summary << "Quality Score: " << j.value("QualityScore", 0) << "%\n";
        summary << "Analysis Duration: " << j.value("AnalysisDurationMs", 0.0) << "ms\n\n";
        
        summary << "Components Found: " << j.value("MonoBehaviourCount", 0) << "\n";
        summary << "Dependencies: " << j.value("DependencyCount", 0) << "\n";
        summary << "Design Patterns: " << j.value("DetectedPatternCount", 0) << "\n\n";
        
        if (j.contains("Components") && j["Components"].is_array()) {
            summary << "Component Details:\n";
            for (const auto& comp : j["Components"]) {
                summary << "- " << comp.value("Name", "Unknown") << " (" << comp.value("BaseClass", "Object") << ")\n";
            }
        }
        
        return AllocateString(summary.str());
        
    } catch (const std::exception& e) {
        SetLastError("Summary generation failed: " + std::string(e.what()));
        return nullptr;
    }
}

UNITY_EXPORT const char* UNITY_CALL GetLastError() {
    std::string error = GetLastErrorInternal();
    return error.empty() ? nullptr : AllocateString(error);
}

UNITY_EXPORT void UNITY_CALL FreeString(const char* str) {
    if (str) {
        delete[] str;
    }
}

UNITY_EXPORT const char* UNITY_CALL GetVersion() {
    return AllocateString("1.0.0");
}

UNITY_EXPORT int UNITY_CALL ValidateProjectPath(const char* projectPath) {
    if (!projectPath) {
        return 0;
    }
    
    try {
        std::filesystem::path path(projectPath);
        return std::filesystem::exists(path) && std::filesystem::is_directory(path) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

UNITY_EXPORT int UNITY_CALL IsUnityProject(const char* projectPath) {
    if (!projectPath) {
        return 0;
    }
    
    try {
        std::filesystem::path path(projectPath);
        
        // Check for typical Unity project structure
        std::filesystem::path assetsPath = path / "Assets";
        std::filesystem::path projectSettingsPath = path.parent_path() / "ProjectSettings";
        
        bool hasAssets = std::filesystem::exists(assetsPath) && std::filesystem::is_directory(assetsPath);
        bool hasProjectSettings = std::filesystem::exists(projectSettingsPath) && std::filesystem::is_directory(projectSettingsPath);
        
        return (hasAssets || hasProjectSettings) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

UNITY_EXPORT void UNITY_CALL SetProgressCallback(ProgressCallback callback) {
    g_progressCallback = callback;
}

}