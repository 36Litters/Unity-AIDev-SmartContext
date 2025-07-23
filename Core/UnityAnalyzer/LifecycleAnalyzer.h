#pragma once

#include "MonoBehaviourAnalyzer.h"
#include <string>
#include <vector>
#include <map>

namespace UnityContextGen {
namespace Unity {

enum class LifecyclePhase {
    Initialization,
    Activation,
    FrameUpdate,
    PhysicsUpdate,
    LateFrameUpdate,
    PhysicsEvent,
    InputEvent,
    RenderingEvent,
    Deactivation,
    Cleanup,
    Custom
};

struct LifecycleMethodInfo {
    std::string method_name;
    std::string class_name;
    LifecyclePhase phase;
    int execution_order;
    std::string purpose;
    std::vector<std::string> interactions;
    size_t line_number;
};

struct LifecycleFlow {
    std::string component_name;
    std::vector<LifecycleMethodInfo> methods;
    std::map<LifecyclePhase, std::vector<std::string>> phase_methods;
    std::vector<std::string> data_flow;
};

class LifecycleAnalyzer {
public:
    LifecycleAnalyzer();
    ~LifecycleAnalyzer() = default;

    void analyzeComponent(const MonoBehaviourInfo& mb_info);
    void analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours);
    
    LifecycleFlow getComponentLifecycle(const std::string& component_name) const;
    std::vector<LifecycleFlow> getAllLifecycleFlows() const;
    
    std::vector<std::string> getMethodsInPhase(LifecyclePhase phase) const;
    std::vector<std::string> getExecutionOrder() const;
    
    std::map<std::string, std::vector<std::string>> getDataFlowAnalysis() const;
    std::string generateLifecycleSummary() const;

private:
    std::map<std::string, LifecycleFlow> m_component_lifecycles;
    
    static const std::map<std::string, LifecyclePhase> METHOD_PHASES;
    static const std::map<std::string, int> METHOD_EXECUTION_ORDER;
    
    LifecyclePhase getMethodPhase(const std::string& method_name) const;
    int getMethodExecutionOrder(const std::string& method_name) const;
    
    std::vector<std::string> analyzeMethodInteractions(const MonoBehaviourInfo& mb_info,
                                                      const std::string& method_name) const;
    
    std::vector<std::string> extractDataFlow(const MonoBehaviourInfo& mb_info) const;
    std::string generatePhaseDescription(LifecyclePhase phase) const;
};

} // namespace Unity
} // namespace UnityContextGen