#include "ComponentMetadata.h"
#include <algorithm>
#include <regex>

namespace UnityContextGen {
namespace Metadata {

ComponentMetadataGenerator::ComponentMetadataGenerator() {
}

void ComponentMetadataGenerator::analyzeComponent(const Unity::MonoBehaviourInfo& mb_info,
                                                 const Unity::ComponentGraph& dependency_graph,
                                                 const Unity::LifecycleFlow& lifecycle_flow,
                                                 const std::vector<Unity::PatternInstance>& patterns) {
    ComponentMetadata metadata;
    
    metadata.class_name = mb_info.class_name;
    metadata.file_path = mb_info.file_path;
    metadata.purpose = inferComponentPurpose(mb_info, patterns);
    metadata.start_line = mb_info.start_line;
    metadata.end_line = mb_info.end_line;
    
    auto deps_it = dependency_graph.dependencies.find(mb_info.class_name);
    if (deps_it != dependency_graph.dependencies.end()) {
        metadata.dependencies = deps_it->second;
    }
    
    auto dependents_it = dependency_graph.dependents.find(mb_info.class_name);
    if (dependents_it != dependency_graph.dependents.end()) {
        metadata.dependents = dependents_it->second;
    }
    
    populateLifecycleInfo(metadata, lifecycle_flow);
    populatePatternInfo(metadata, patterns);
    generateResponsibilityBlocks(metadata, mb_info);
    
    metadata.serialized_fields = mb_info.serialized_fields;
    metadata.attributes = mb_info.attributes;
    
    calculateMetrics(metadata, mb_info, dependency_graph);
    
    m_component_metadata[mb_info.class_name] = metadata;
}

void ComponentMetadataGenerator::analyzeProject(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                                               const Unity::ComponentGraph& dependency_graph,
                                               const std::vector<Unity::LifecycleFlow>& lifecycle_flows,
                                               const std::vector<Unity::PatternInstance>& patterns) {
    m_component_metadata.clear();
    
    for (const auto& mb_info : monobehaviours) {
        Unity::LifecycleFlow lifecycle_flow;
        for (const auto& flow : lifecycle_flows) {
            if (flow.component_name == mb_info.class_name) {
                lifecycle_flow = flow;
                break;
            }
        }
        
        analyzeComponent(mb_info, dependency_graph, lifecycle_flow, patterns);
    }
}

void ComponentMetadataGenerator::populateLifecycleInfo(ComponentMetadata& metadata,
                                                      const Unity::LifecycleFlow& lifecycle_flow) const {
    for (const auto& method : lifecycle_flow.methods) {
        metadata.lifecycle.methods.push_back(method.method_name);
        metadata.lifecycle.method_purposes[method.method_name] = method.purpose;
    }
    
    std::sort(metadata.lifecycle.methods.begin(), metadata.lifecycle.methods.end());
    
    std::vector<Unity::LifecycleMethodInfo> sorted_methods = lifecycle_flow.methods;
    std::sort(sorted_methods.begin(), sorted_methods.end(),
              [](const Unity::LifecycleMethodInfo& a, const Unity::LifecycleMethodInfo& b) {
                  return a.execution_order < b.execution_order;
              });
    
    for (const auto& method : sorted_methods) {
        metadata.lifecycle.execution_order.push_back(method.method_name);
    }
    
    metadata.lifecycle.data_flow = lifecycle_flow.data_flow;
}

void ComponentMetadataGenerator::populatePatternInfo(ComponentMetadata& metadata,
                                                    const std::vector<Unity::PatternInstance>& patterns) const {
    float max_confidence = 0.0f;
    
    for (const auto& pattern : patterns) {
        auto it = std::find(pattern.involved_components.begin(),
                           pattern.involved_components.end(),
                           metadata.class_name);
        
        if (it != pattern.involved_components.end()) {
            metadata.design_patterns.push_back(pattern.pattern_name);
            if (pattern.confidence_score > max_confidence) {
                max_confidence = pattern.confidence_score;
            }
        }
    }
    
    metadata.metrics.pattern_confidence = max_confidence;
}

void ComponentMetadataGenerator::generateResponsibilityBlocks(ComponentMetadata& metadata,
                                                             const Unity::MonoBehaviourInfo& mb_info) const {
    std::map<std::string, std::vector<std::string>> responsibility_groups;
    
    for (const auto& method : mb_info.unity_methods) {
        std::string category = categorizeResponsibility(method);
        responsibility_groups[category].push_back(method);
    }
    
    for (const auto& method : mb_info.custom_methods) {
        std::string category = categorizeResponsibility(method);
        responsibility_groups[category].push_back(method);
    }
    
    for (const auto& group : responsibility_groups) {
        if (!group.second.empty()) {
            std::string methods_list;
            for (size_t i = 0; i < group.second.size(); ++i) {
                if (i > 0) methods_list += ", ";
                methods_list += group.second[i];
            }
            
            metadata.responsibility_blocks.blocks[group.first] = methods_list;
            metadata.responsibility_blocks.block_order.push_back(group.first);
        }
    }
    
    std::sort(metadata.responsibility_blocks.block_order.begin(),
              metadata.responsibility_blocks.block_order.end());
}

std::string ComponentMetadataGenerator::categorizeResponsibility(const std::string& method_name) const {
    if (method_name == "Awake" || method_name == "Start") {
        return "Initialization";
    } else if (method_name == "Update") {
        return "Input Handling";
    } else if (method_name == "FixedUpdate") {
        return "Physics";
    } else if (method_name == "LateUpdate") {
        return "Late Update";
    } else if (method_name.find("OnTrigger") == 0 || method_name.find("OnCollision") == 0) {
        return "Physics Events";
    } else if (method_name.find("OnMouse") == 0) {
        return "Input Events";
    } else if (method_name.find("OnRender") != std::string::npos || 
               method_name.find("OnDraw") != std::string::npos) {
        return "Rendering";
    } else if (method_name == "OnEnable" || method_name == "OnDisable" || method_name == "OnDestroy") {
        return "Lifecycle Management";
    } else if (method_name.find("Move") != std::string::npos || 
               method_name.find("Jump") != std::string::npos ||
               method_name.find("Walk") != std::string::npos ||
               method_name.find("Run") != std::string::npos) {
        return "Movement";
    } else if (method_name.find("Attack") != std::string::npos || 
               method_name.find("Shoot") != std::string::npos ||
               method_name.find("Fire") != std::string::npos) {
        return "Combat";
    } else if (method_name.find("UI") != std::string::npos || 
               method_name.find("Menu") != std::string::npos ||
               method_name.find("Button") != std::string::npos) {
        return "UI Handling";
    } else if (method_name.find("Audio") != std::string::npos || 
               method_name.find("Sound") != std::string::npos ||
               method_name.find("Music") != std::string::npos) {
        return "Audio";
    } else if (method_name.find("AI") != std::string::npos || 
               method_name.find("Behavior") != std::string::npos ||
               method_name.find("Decision") != std::string::npos) {
        return "AI Logic";
    }
    
    return "Custom Logic";
}

std::string ComponentMetadataGenerator::inferComponentPurpose(const Unity::MonoBehaviourInfo& mb_info,
                                                            const std::vector<Unity::PatternInstance>& patterns) const {
    std::vector<std::string> keywords = extractResponsibilityKeywords(mb_info);
    
    if (std::find(keywords.begin(), keywords.end(), "Player") != keywords.end()) {
        if (std::find(keywords.begin(), keywords.end(), "Controller") != keywords.end()) {
            return "Handles player movement, input, and interactions";
        } else if (std::find(keywords.begin(), keywords.end(), "Health") != keywords.end()) {
            return "Manages player health and damage systems";
        }
        return "Manages player-related functionality";
    } else if (std::find(keywords.begin(), keywords.end(), "Enemy") != keywords.end()) {
        return "Controls enemy behavior, AI, and interactions";
    } else if (std::find(keywords.begin(), keywords.end(), "Camera") != keywords.end()) {
        return "Controls camera movement and behavior";
    } else if (std::find(keywords.begin(), keywords.end(), "UI") != keywords.end()) {
        return "Manages user interface elements and interactions";
    } else if (std::find(keywords.begin(), keywords.end(), "Manager") != keywords.end()) {
        return "Manages game systems and coordinates functionality";
    } else if (std::find(keywords.begin(), keywords.end(), "Controller") != keywords.end()) {
        return "Controls object behavior and state management";
    }
    
    if (!mb_info.unity_methods.empty()) {
        bool has_physics = std::find(mb_info.unity_methods.begin(), mb_info.unity_methods.end(), "FixedUpdate") 
                          != mb_info.unity_methods.end();
        bool has_input = std::find(mb_info.unity_methods.begin(), mb_info.unity_methods.end(), "Update") 
                        != mb_info.unity_methods.end();
        bool has_collision = false;
        
        for (const auto& method : mb_info.unity_methods) {
            if (method.find("OnTrigger") == 0 || method.find("OnCollision") == 0) {
                has_collision = true;
                break;
            }
        }
        
        if (has_physics && has_input) {
            return "Handles object movement and physics-based interactions";
        } else if (has_collision) {
            return "Manages collision detection and response";
        } else if (has_input) {
            return "Processes input and updates object state";
        } else if (has_physics) {
            return "Manages physics-based behavior";
        }
    }
    
    for (const auto& pattern : patterns) {
        auto it = std::find(pattern.involved_components.begin(),
                           pattern.involved_components.end(),
                           mb_info.class_name);
        if (it != pattern.involved_components.end()) {
            return pattern.purpose;
        }
    }
    
    return "Unity MonoBehaviour component";
}

std::vector<std::string> ComponentMetadataGenerator::extractResponsibilityKeywords(const Unity::MonoBehaviourInfo& mb_info) const {
    std::vector<std::string> keywords;
    
    std::regex word_regex(R"([A-Z][a-z]+)");
    std::sregex_iterator iter(mb_info.class_name.begin(), mb_info.class_name.end(), word_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::smatch match = *iter;
        keywords.push_back(match.str());
    }
    
    return keywords;
}

void ComponentMetadataGenerator::calculateMetrics(ComponentMetadata& metadata,
                                                 const Unity::MonoBehaviourInfo& mb_info,
                                                 const Unity::ComponentGraph& dependency_graph) const {
    metadata.metrics.dependency_count = mb_info.component_dependencies.size();
    metadata.metrics.method_count = mb_info.unity_methods.size() + mb_info.custom_methods.size();
    metadata.metrics.unity_method_count = mb_info.unity_methods.size();
    
    int complexity = 0;
    complexity += mb_info.unity_methods.size() * 2;
    complexity += mb_info.custom_methods.size();
    complexity += mb_info.component_dependencies.size() * 3;
    complexity += mb_info.serialized_fields.size();
    
    metadata.metrics.complexity_score = complexity;
}

ComponentMetadata ComponentMetadataGenerator::getComponentMetadata(const std::string& component_name) const {
    auto it = m_component_metadata.find(component_name);
    if (it != m_component_metadata.end()) {
        return it->second;
    }
    return ComponentMetadata();
}

std::vector<ComponentMetadata> ComponentMetadataGenerator::getAllComponentMetadata() const {
    std::vector<ComponentMetadata> metadata_list;
    for (const auto& pair : m_component_metadata) {
        metadata_list.push_back(pair.second);
    }
    return metadata_list;
}

nlohmann::json ComponentMetadataGenerator::exportToJSON() const {
    nlohmann::json json_output;
    json_output["components"] = nlohmann::json::object();
    
    for (const auto& pair : m_component_metadata) {
        const auto& metadata = pair.second;
        
        nlohmann::json component_json;
        component_json["class_name"] = metadata.class_name;
        component_json["file_path"] = metadata.file_path;
        component_json["purpose"] = metadata.purpose;
        component_json["dependencies"] = metadata.dependencies;
        component_json["dependents"] = metadata.dependents;
        
        component_json["lifecycle"] = {
            {"methods", metadata.lifecycle.methods},
            {"method_purposes", metadata.lifecycle.method_purposes},
            {"execution_order", metadata.lifecycle.execution_order},
            {"data_flow", metadata.lifecycle.data_flow}
        };
        
        component_json["responsibility_blocks"] = {
            {"blocks", metadata.responsibility_blocks.blocks},
            {"block_order", metadata.responsibility_blocks.block_order}
        };
        
        component_json["serialized_fields"] = metadata.serialized_fields;
        component_json["attributes"] = metadata.attributes;
        component_json["design_patterns"] = metadata.design_patterns;
        
        component_json["metrics"] = {
            {"complexity_score", metadata.metrics.complexity_score},
            {"dependency_count", metadata.metrics.dependency_count},
            {"method_count", metadata.metrics.method_count},
            {"unity_method_count", metadata.metrics.unity_method_count},
            {"pattern_confidence", metadata.metrics.pattern_confidence}
        };
        
        component_json["location"] = {
            {"start_line", metadata.start_line},
            {"end_line", metadata.end_line}
        };
        
        json_output["components"][metadata.class_name] = component_json;
    }
    
    return json_output;
}

nlohmann::json ComponentMetadataGenerator::exportComponentToJSON(const std::string& component_name) const {
    auto metadata = getComponentMetadata(component_name);
    if (metadata.class_name.empty()) {
        return nlohmann::json::object();
    }
    
    nlohmann::json component_json;
    component_json["class_name"] = metadata.class_name;
    component_json["file_path"] = metadata.file_path;
    component_json["purpose"] = metadata.purpose;
    component_json["dependencies"] = metadata.dependencies;
    component_json["dependents"] = metadata.dependents;
    
    component_json["lifecycle"] = {
        {"methods", metadata.lifecycle.methods},
        {"method_purposes", metadata.lifecycle.method_purposes},
        {"execution_order", metadata.lifecycle.execution_order},
        {"data_flow", metadata.lifecycle.data_flow}
    };
    
    component_json["responsibility_blocks"] = {
        {"blocks", metadata.responsibility_blocks.blocks},
        {"block_order", metadata.responsibility_blocks.block_order}
    };
    
    component_json["serialized_fields"] = metadata.serialized_fields;
    component_json["attributes"] = metadata.attributes;
    component_json["design_patterns"] = metadata.design_patterns;
    
    component_json["metrics"] = {
        {"complexity_score", metadata.metrics.complexity_score},
        {"dependency_count", metadata.metrics.dependency_count},
        {"method_count", metadata.metrics.method_count},
        {"unity_method_count", metadata.metrics.unity_method_count},
        {"pattern_confidence", metadata.metrics.pattern_confidence}
    };
    
    component_json["location"] = {
        {"start_line", metadata.start_line},
        {"end_line", metadata.end_line}
    };
    
    return component_json;
}

} // namespace Metadata
} // namespace UnityContextGen