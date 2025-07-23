#pragma once

#include "../TreeSitterEngine/CSharpParser.h"
#include <string>
#include <vector>
#include <map>
#include <set>

namespace UnityContextGen {
namespace Unity {

struct MonoBehaviourInfo {
    std::string class_name;
    std::string file_path;
    std::vector<std::string> unity_methods;
    std::vector<std::string> custom_methods;
    std::vector<std::string> serialized_fields;
    std::vector<std::string> component_dependencies;
    std::vector<std::string> attributes;
    std::map<std::string, std::string> method_purposes;
    size_t start_line;
    size_t end_line;
};

struct UnityMethodInfo {
    std::string name;
    std::string class_name;
    std::string purpose;
    std::vector<std::string> dependencies;
    std::vector<std::string> calls_to_other_methods;
    size_t line_number;
    std::string execution_phase;
};

class MonoBehaviourAnalyzer {
public:
    MonoBehaviourAnalyzer();
    ~MonoBehaviourAnalyzer() = default;

    bool analyzeFile(const std::string& file_path);
    bool analyzeCode(const std::string& source_code, const std::string& file_path = "");
    
    std::vector<MonoBehaviourInfo> getMonoBehaviours() const;
    std::vector<UnityMethodInfo> getUnityMethods() const;
    
    bool isMonoBehaviour(const TreeSitter::ClassInfo& class_info) const;
    bool isUnityMethod(const std::string& method_name) const;
    
    std::string getMethodExecutionPhase(const std::string& method_name) const;
    std::vector<std::string> extractComponentDependencies(const std::string& source_code) const;
    std::vector<std::string> extractSerializedFields(const std::vector<TreeSitter::FieldInfo>& fields) const;

private:
    TreeSitter::CSharpParser m_parser;
    std::vector<MonoBehaviourInfo> m_monobehaviours;
    std::vector<UnityMethodInfo> m_unity_methods;
    
    static const std::set<std::string> UNITY_LIFECYCLE_METHODS;
    static const std::set<std::string> UNITY_EVENT_METHODS;
    static const std::set<std::string> UNITY_PHYSICS_METHODS;
    static const std::set<std::string> UNITY_RENDERING_METHODS;
    static const std::set<std::string> UNITY_INPUT_METHODS;
    
    static const std::map<std::string, std::string> METHOD_EXECUTION_PHASES;
    
    void analyzeMonoBehaviour(const TreeSitter::ClassInfo& class_info, 
                             const std::vector<TreeSitter::MethodInfo>& methods,
                             const std::vector<TreeSitter::FieldInfo>& fields,
                             const std::string& file_path);
    
    std::string inferMethodPurpose(const std::string& method_name, 
                                  const TreeSitter::MethodInfo& method_info) const;
    
    std::vector<std::string> extractGetComponentCalls(const std::string& source_code) const;
    std::vector<std::string> extractRequireComponentAttributes(const std::vector<std::string>& attributes) const;
    
    bool hasUnityAttribute(const std::vector<std::string>& attributes, const std::string& attr_name) const;
};

} // namespace Unity
} // namespace UnityContextGen