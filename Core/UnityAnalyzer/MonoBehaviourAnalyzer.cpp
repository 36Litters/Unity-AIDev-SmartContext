#include "MonoBehaviourAnalyzer.h"
#include <algorithm>
#include <regex>
#include <iostream>

namespace UnityContextGen {
namespace Unity {

const std::set<std::string> MonoBehaviourAnalyzer::UNITY_LIFECYCLE_METHODS = {
    "Awake", "Start", "Update", "FixedUpdate", "LateUpdate", 
    "OnEnable", "OnDisable", "OnDestroy"
};

const std::set<std::string> MonoBehaviourAnalyzer::UNITY_EVENT_METHODS = {
    "OnTriggerEnter", "OnTriggerExit", "OnTriggerStay",
    "OnCollisionEnter", "OnCollisionExit", "OnCollisionStay",
    "OnTriggerEnter2D", "OnTriggerExit2D", "OnTriggerStay2D",
    "OnCollisionEnter2D", "OnCollisionExit2D", "OnCollisionStay2D"
};

const std::set<std::string> MonoBehaviourAnalyzer::UNITY_PHYSICS_METHODS = {
    "FixedUpdate", "OnCollisionEnter", "OnCollisionExit", "OnCollisionStay",
    "OnTriggerEnter", "OnTriggerExit", "OnTriggerStay",
    "OnCollisionEnter2D", "OnCollisionExit2D", "OnCollisionStay2D",
    "OnTriggerEnter2D", "OnTriggerExit2D", "OnTriggerStay2D"
};

const std::set<std::string> MonoBehaviourAnalyzer::UNITY_RENDERING_METHODS = {
    "OnPreRender", "OnPostRender", "OnRenderObject", "OnWillRenderObject",
    "OnBecameVisible", "OnBecameInvisible", "OnDrawGizmos", "OnDrawGizmosSelected"
};

const std::set<std::string> MonoBehaviourAnalyzer::UNITY_INPUT_METHODS = {
    "OnMouseDown", "OnMouseUp", "OnMouseEnter", "OnMouseExit", "OnMouseOver",
    "OnMouseDrag", "OnMouseUpAsButton"
};

const std::map<std::string, std::string> MonoBehaviourAnalyzer::METHOD_EXECUTION_PHASES = {
    {"Awake", "Initialization"},
    {"Start", "Initialization"},
    {"OnEnable", "Activation"},
    {"Update", "Frame Update"},
    {"FixedUpdate", "Physics Update"},
    {"LateUpdate", "Late Frame Update"},
    {"OnDisable", "Deactivation"},
    {"OnDestroy", "Cleanup"},
    {"OnTriggerEnter", "Physics Event"},
    {"OnTriggerExit", "Physics Event"},
    {"OnTriggerStay", "Physics Event"},
    {"OnCollisionEnter", "Physics Event"},
    {"OnCollisionExit", "Physics Event"},
    {"OnCollisionStay", "Physics Event"}
};

MonoBehaviourAnalyzer::MonoBehaviourAnalyzer() {
}

bool MonoBehaviourAnalyzer::analyzeFile(const std::string& file_path) {
    m_monobehaviours.clear();
    m_unity_methods.clear();
    
    if (!m_parser.parseFile(file_path)) {
        return false;
    }
    
    auto classes = m_parser.getClasses();
    auto methods = m_parser.getMethods();
    auto fields = m_parser.getFields();
    
    for (const auto& class_info : classes) {
        if (isMonoBehaviour(class_info)) {
            analyzeMonoBehaviour(class_info, methods, fields, file_path);
        }
    }
    
    return true;
}

bool MonoBehaviourAnalyzer::analyzeCode(const std::string& source_code, const std::string& file_path) {
    m_monobehaviours.clear();
    m_unity_methods.clear();
    
    if (!m_parser.parseCode(source_code)) {
        return false;
    }
    
    auto classes = m_parser.getClasses();
    auto methods = m_parser.getMethods();
    auto fields = m_parser.getFields();
    
    for (const auto& class_info : classes) {
        if (isMonoBehaviour(class_info)) {
            analyzeMonoBehaviour(class_info, methods, fields, file_path);
        }
    }
    
    return true;
}

bool MonoBehaviourAnalyzer::isMonoBehaviour(const TreeSitter::ClassInfo& class_info) const {
    return class_info.base_class == "MonoBehaviour" || 
           class_info.base_class == "UnityEngine.MonoBehaviour";
}

bool MonoBehaviourAnalyzer::isUnityMethod(const std::string& method_name) const {
    return UNITY_LIFECYCLE_METHODS.count(method_name) > 0 ||
           UNITY_EVENT_METHODS.count(method_name) > 0 ||
           UNITY_PHYSICS_METHODS.count(method_name) > 0 ||
           UNITY_RENDERING_METHODS.count(method_name) > 0 ||
           UNITY_INPUT_METHODS.count(method_name) > 0;
}

std::string MonoBehaviourAnalyzer::getMethodExecutionPhase(const std::string& method_name) const {
    auto it = METHOD_EXECUTION_PHASES.find(method_name);
    if (it != METHOD_EXECUTION_PHASES.end()) {
        return it->second;
    }
    return "Custom";
}

void MonoBehaviourAnalyzer::analyzeMonoBehaviour(const TreeSitter::ClassInfo& class_info,
                                                const std::vector<TreeSitter::MethodInfo>& methods,
                                                const std::vector<TreeSitter::FieldInfo>& fields,
                                                const std::string& file_path) {
    MonoBehaviourInfo mb_info;
    mb_info.class_name = class_info.name;
    mb_info.file_path = file_path;
    mb_info.attributes = class_info.attributes;
    mb_info.start_line = class_info.start_line;
    mb_info.end_line = class_info.end_line;
    
    for (const auto& method : methods) {
        if (isUnityMethod(method.name)) {
            mb_info.unity_methods.push_back(method.name);
            mb_info.method_purposes[method.name] = inferMethodPurpose(method.name, method);
            
            UnityMethodInfo unity_method;
            unity_method.name = method.name;
            unity_method.class_name = class_info.name;
            unity_method.purpose = mb_info.method_purposes[method.name];
            unity_method.line_number = method.start_line;
            unity_method.execution_phase = getMethodExecutionPhase(method.name);
            
            m_unity_methods.push_back(unity_method);
        } else {
            mb_info.custom_methods.push_back(method.name);
        }
    }
    
    mb_info.serialized_fields = extractSerializedFields(fields);
    
    std::string source_code = m_parser.getSourceCode();
    mb_info.component_dependencies = extractComponentDependencies(source_code);
    
    auto required_components = extractRequireComponentAttributes(class_info.attributes);
    mb_info.component_dependencies.insert(mb_info.component_dependencies.end(),
                                         required_components.begin(),
                                         required_components.end());
    
    m_monobehaviours.push_back(mb_info);
}

std::string MonoBehaviourAnalyzer::inferMethodPurpose(const std::string& method_name,
                                                     const TreeSitter::MethodInfo& method_info) const {
    if (method_name == "Awake") {
        return "Initialize component references and setup";
    } else if (method_name == "Start") {
        return "Initialize after all objects are created";
    } else if (method_name == "Update") {
        return "Handle per-frame logic and input";
    } else if (method_name == "FixedUpdate") {
        return "Handle physics and fixed-timestep logic";
    } else if (method_name == "LateUpdate") {
        return "Handle logic after all Update calls";
    } else if (method_name == "OnEnable") {
        return "Handle component activation";
    } else if (method_name == "OnDisable") {
        return "Handle component deactivation";
    } else if (method_name == "OnDestroy") {
        return "Cleanup resources and references";
    } else if (method_name.find("OnTrigger") == 0) {
        return "Handle trigger collision events";
    } else if (method_name.find("OnCollision") == 0) {
        return "Handle physics collision events";
    } else if (method_name.find("OnMouse") == 0) {
        return "Handle mouse input events";
    }
    
    return "Unity callback method";
}

std::vector<std::string> MonoBehaviourAnalyzer::extractComponentDependencies(const std::string& source_code) const {
    std::vector<std::string> dependencies;
    
    auto get_component_calls = extractGetComponentCalls(source_code);
    dependencies.insert(dependencies.end(), get_component_calls.begin(), get_component_calls.end());
    
    return dependencies;
}

std::vector<std::string> MonoBehaviourAnalyzer::extractGetComponentCalls(const std::string& source_code) const {
    std::vector<std::string> components;
    
    std::regex get_component_regex(R"(GetComponent<(\w+)>\(\))");
    std::regex get_component_in_children_regex(R"(GetComponentInChildren<(\w+)>\(\))");
    std::regex get_component_in_parent_regex(R"(GetComponentInParent<(\w+)>\(\))");
    
    std::sregex_iterator iter(source_code.begin(), source_code.end(), get_component_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        components.push_back(match[1].str());
    }
    
    iter = std::sregex_iterator(source_code.begin(), source_code.end(), get_component_in_children_regex);
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        components.push_back(match[1].str());
    }
    
    iter = std::sregex_iterator(source_code.begin(), source_code.end(), get_component_in_parent_regex);
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        components.push_back(match[1].str());
    }
    
    std::sort(components.begin(), components.end());
    components.erase(std::unique(components.begin(), components.end()), components.end());
    
    return components;
}

std::vector<std::string> MonoBehaviourAnalyzer::extractRequireComponentAttributes(const std::vector<std::string>& attributes) const {
    std::vector<std::string> components;
    
    for (const auto& attr : attributes) {
        if (attr.find("RequireComponent") != std::string::npos) {
            std::regex require_component_regex(R"(RequireComponent\(typeof\((\w+)\)\))");
            std::smatch match;
            if (std::regex_search(attr, match, require_component_regex)) {
                components.push_back(match[1].str());
            }
        }
    }
    
    return components;
}

std::vector<std::string> MonoBehaviourAnalyzer::extractSerializedFields(const std::vector<TreeSitter::FieldInfo>& fields) const {
    std::vector<std::string> serialized_fields;
    
    for (const auto& field : fields) {
        if (field.access_modifier == "public" || 
            hasUnityAttribute(field.attributes, "SerializeField")) {
            serialized_fields.push_back(field.name + " : " + field.type);
        }
    }
    
    return serialized_fields;
}

bool MonoBehaviourAnalyzer::hasUnityAttribute(const std::vector<std::string>& attributes, const std::string& attr_name) const {
    for (const auto& attr : attributes) {
        if (attr.find(attr_name) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<MonoBehaviourInfo> MonoBehaviourAnalyzer::getMonoBehaviours() const {
    return m_monobehaviours;
}

std::vector<UnityMethodInfo> MonoBehaviourAnalyzer::getUnityMethods() const {
    return m_unity_methods;
}

} // namespace Unity
} // namespace UnityContextGen