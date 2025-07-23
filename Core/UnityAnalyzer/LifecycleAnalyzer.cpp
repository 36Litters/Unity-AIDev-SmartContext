#include "LifecycleAnalyzer.h"
#include <algorithm>

namespace UnityContextGen {
namespace Unity {

const std::map<std::string, LifecyclePhase> LifecycleAnalyzer::METHOD_PHASES = {
    {"Awake", LifecyclePhase::Initialization},
    {"Start", LifecyclePhase::Initialization},
    {"OnEnable", LifecyclePhase::Activation},
    {"Update", LifecyclePhase::FrameUpdate},
    {"FixedUpdate", LifecyclePhase::PhysicsUpdate},
    {"LateUpdate", LifecyclePhase::LateFrameUpdate},
    {"OnTriggerEnter", LifecyclePhase::PhysicsEvent},
    {"OnTriggerExit", LifecyclePhase::PhysicsEvent},
    {"OnTriggerStay", LifecyclePhase::PhysicsEvent},
    {"OnCollisionEnter", LifecyclePhase::PhysicsEvent},
    {"OnCollisionExit", LifecyclePhase::PhysicsEvent},
    {"OnCollisionStay", LifecyclePhase::PhysicsEvent},
    {"OnTriggerEnter2D", LifecyclePhase::PhysicsEvent},
    {"OnTriggerExit2D", LifecyclePhase::PhysicsEvent},
    {"OnTriggerStay2D", LifecyclePhase::PhysicsEvent},
    {"OnCollisionEnter2D", LifecyclePhase::PhysicsEvent},
    {"OnCollisionExit2D", LifecyclePhase::PhysicsEvent},
    {"OnCollisionStay2D", LifecyclePhase::PhysicsEvent},
    {"OnMouseDown", LifecyclePhase::InputEvent},
    {"OnMouseUp", LifecyclePhase::InputEvent},
    {"OnMouseEnter", LifecyclePhase::InputEvent},
    {"OnMouseExit", LifecyclePhase::InputEvent},
    {"OnMouseOver", LifecyclePhase::InputEvent},
    {"OnMouseDrag", LifecyclePhase::InputEvent},
    {"OnMouseUpAsButton", LifecyclePhase::InputEvent},
    {"OnPreRender", LifecyclePhase::RenderingEvent},
    {"OnPostRender", LifecyclePhase::RenderingEvent},
    {"OnRenderObject", LifecyclePhase::RenderingEvent},
    {"OnWillRenderObject", LifecyclePhase::RenderingEvent},
    {"OnBecameVisible", LifecyclePhase::RenderingEvent},
    {"OnBecameInvisible", LifecyclePhase::RenderingEvent},
    {"OnDrawGizmos", LifecyclePhase::RenderingEvent},
    {"OnDrawGizmosSelected", LifecyclePhase::RenderingEvent},
    {"OnDisable", LifecyclePhase::Deactivation},
    {"OnDestroy", LifecyclePhase::Cleanup}
};

const std::map<std::string, int> LifecycleAnalyzer::METHOD_EXECUTION_ORDER = {
    {"Awake", 1},
    {"OnEnable", 2},
    {"Start", 3},
    {"Update", 100},
    {"FixedUpdate", 101},
    {"LateUpdate", 102},
    {"OnTriggerEnter", 200},
    {"OnTriggerStay", 201},
    {"OnTriggerExit", 202},
    {"OnCollisionEnter", 203},
    {"OnCollisionStay", 204},
    {"OnCollisionExit", 205},
    {"OnMouseDown", 300},
    {"OnMouseUp", 301},
    {"OnMouseEnter", 302},
    {"OnMouseExit", 303},
    {"OnMouseOver", 304},
    {"OnMouseDrag", 305},
    {"OnMouseUpAsButton", 306},
    {"OnPreRender", 400},
    {"OnWillRenderObject", 401},
    {"OnRenderObject", 402},
    {"OnPostRender", 403},
    {"OnBecameVisible", 404},
    {"OnBecameInvisible", 405},
    {"OnDrawGizmos", 406},
    {"OnDrawGizmosSelected", 407},
    {"OnDisable", 500},
    {"OnDestroy", 501}
};

LifecycleAnalyzer::LifecycleAnalyzer() {
}

void LifecycleAnalyzer::analyzeComponent(const MonoBehaviourInfo& mb_info) {
    LifecycleFlow flow;
    flow.component_name = mb_info.class_name;
    
    for (const auto& method_name : mb_info.unity_methods) {
        LifecycleMethodInfo method_info;
        method_info.method_name = method_name;
        method_info.class_name = mb_info.class_name;
        method_info.phase = getMethodPhase(method_name);
        method_info.execution_order = getMethodExecutionOrder(method_name);
        
        auto purpose_it = mb_info.method_purposes.find(method_name);
        if (purpose_it != mb_info.method_purposes.end()) {
            method_info.purpose = purpose_it->second;
        }
        
        method_info.interactions = analyzeMethodInteractions(mb_info, method_name);
        method_info.line_number = 0;
        
        flow.methods.push_back(method_info);
        flow.phase_methods[method_info.phase].push_back(method_name);
    }
    
    std::sort(flow.methods.begin(), flow.methods.end(), 
              [](const LifecycleMethodInfo& a, const LifecycleMethodInfo& b) {
                  return a.execution_order < b.execution_order;
              });
    
    flow.data_flow = extractDataFlow(mb_info);
    
    m_component_lifecycles[mb_info.class_name] = flow;
}

void LifecycleAnalyzer::analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours) {
    m_component_lifecycles.clear();
    
    for (const auto& mb_info : monobehaviours) {
        analyzeComponent(mb_info);
    }
}

LifecyclePhase LifecycleAnalyzer::getMethodPhase(const std::string& method_name) const {
    auto it = METHOD_PHASES.find(method_name);
    if (it != METHOD_PHASES.end()) {
        return it->second;
    }
    return LifecyclePhase::Custom;
}

int LifecycleAnalyzer::getMethodExecutionOrder(const std::string& method_name) const {
    auto it = METHOD_EXECUTION_ORDER.find(method_name);
    if (it != METHOD_EXECUTION_ORDER.end()) {
        return it->second;
    }
    return 1000;
}

std::vector<std::string> LifecycleAnalyzer::analyzeMethodInteractions(const MonoBehaviourInfo& mb_info,
                                                                     const std::string& method_name) const {
    std::vector<std::string> interactions;
    
    for (const auto& dep : mb_info.component_dependencies) {
        interactions.push_back("Interacts with " + dep);
    }
    
    if (method_name == "Awake") {
        interactions.push_back("Initializes component references");
        interactions.push_back("Sets up initial state");
    } else if (method_name == "Start") {
        interactions.push_back("Accesses other initialized components");
        interactions.push_back("Starts component behaviors");
    } else if (method_name == "Update") {
        interactions.push_back("Processes input");
        interactions.push_back("Updates game state");
    } else if (method_name == "FixedUpdate") {
        interactions.push_back("Applies physics forces");
        interactions.push_back("Updates rigidbody properties");
    }
    
    return interactions;
}

std::vector<std::string> LifecycleAnalyzer::extractDataFlow(const MonoBehaviourInfo& mb_info) const {
    std::vector<std::string> data_flow;
    
    bool has_input = false;
    bool has_physics = false;
    bool has_rendering = false;
    
    for (const auto& method : mb_info.unity_methods) {
        if (method == "Update") {
            has_input = true;
            data_flow.push_back("Input Processing");
        } else if (method == "FixedUpdate") {
            has_physics = true;
            data_flow.push_back("Physics Update");
        } else if (method.find("OnRender") != std::string::npos || 
                   method.find("OnDraw") != std::string::npos) {
            has_rendering = true;
            data_flow.push_back("Rendering");
        }
    }
    
    if (has_input && has_physics) {
        data_flow.push_back("Input → Physics");
    }
    if (has_physics && has_rendering) {
        data_flow.push_back("Physics → Rendering");
    }
    
    return data_flow;
}

LifecycleFlow LifecycleAnalyzer::getComponentLifecycle(const std::string& component_name) const {
    auto it = m_component_lifecycles.find(component_name);
    if (it != m_component_lifecycles.end()) {
        return it->second;
    }
    return LifecycleFlow();
}

std::vector<LifecycleFlow> LifecycleAnalyzer::getAllLifecycleFlows() const {
    std::vector<LifecycleFlow> flows;
    for (const auto& pair : m_component_lifecycles) {
        flows.push_back(pair.second);
    }
    return flows;
}

std::vector<std::string> LifecycleAnalyzer::getMethodsInPhase(LifecyclePhase phase) const {
    std::vector<std::string> methods;
    
    for (const auto& pair : m_component_lifecycles) {
        const auto& flow = pair.second;
        for (const auto& method : flow.methods) {
            if (method.phase == phase) {
                methods.push_back(pair.first + "::" + method.method_name);
            }
        }
    }
    
    return methods;
}

std::vector<std::string> LifecycleAnalyzer::getExecutionOrder() const {
    std::vector<LifecycleMethodInfo> all_methods;
    
    for (const auto& pair : m_component_lifecycles) {
        const auto& flow = pair.second;
        all_methods.insert(all_methods.end(), flow.methods.begin(), flow.methods.end());
    }
    
    std::sort(all_methods.begin(), all_methods.end(),
              [](const LifecycleMethodInfo& a, const LifecycleMethodInfo& b) {
                  return a.execution_order < b.execution_order;
              });
    
    std::vector<std::string> execution_order;
    for (const auto& method : all_methods) {
        execution_order.push_back(method.class_name + "::" + method.method_name);
    }
    
    return execution_order;
}

std::map<std::string, std::vector<std::string>> LifecycleAnalyzer::getDataFlowAnalysis() const {
    std::map<std::string, std::vector<std::string>> data_flows;
    
    for (const auto& pair : m_component_lifecycles) {
        data_flows[pair.first] = pair.second.data_flow;
    }
    
    return data_flows;
}

std::string LifecycleAnalyzer::generateLifecycleSummary() const {
    std::string summary = "Unity Lifecycle Analysis Summary:\n\n";
    
    for (const auto& pair : m_component_lifecycles) {
        const auto& flow = pair.second;
        summary += "Component: " + flow.component_name + "\n";
        summary += "Lifecycle Methods: " + std::to_string(flow.methods.size()) + "\n";
        
        for (const auto& phase_pair : flow.phase_methods) {
            if (!phase_pair.second.empty()) {
                summary += "  " + generatePhaseDescription(phase_pair.first) + ": ";
                for (size_t i = 0; i < phase_pair.second.size(); ++i) {
                    if (i > 0) summary += ", ";
                    summary += phase_pair.second[i];
                }
                summary += "\n";
            }
        }
        
        if (!flow.data_flow.empty()) {
            summary += "Data Flow: ";
            for (size_t i = 0; i < flow.data_flow.size(); ++i) {
                if (i > 0) summary += " → ";
                summary += flow.data_flow[i];
            }
            summary += "\n";
        }
        
        summary += "\n";
    }
    
    return summary;
}

std::string LifecycleAnalyzer::generatePhaseDescription(LifecyclePhase phase) const {
    switch (phase) {
        case LifecyclePhase::Initialization: return "Initialization";
        case LifecyclePhase::Activation: return "Activation";
        case LifecyclePhase::FrameUpdate: return "Frame Update";
        case LifecyclePhase::PhysicsUpdate: return "Physics Update";
        case LifecyclePhase::LateFrameUpdate: return "Late Frame Update";
        case LifecyclePhase::PhysicsEvent: return "Physics Events";
        case LifecyclePhase::InputEvent: return "Input Events";
        case LifecyclePhase::RenderingEvent: return "Rendering Events";
        case LifecyclePhase::Deactivation: return "Deactivation";
        case LifecyclePhase::Cleanup: return "Cleanup";
        case LifecyclePhase::Custom: return "Custom";
        default: return "Unknown";
    }
}

} // namespace Unity
} // namespace UnityContextGen