#pragma once

#include "MonoBehaviourAnalyzer.h"
#include <string>
#include <vector>
#include <map>
#include <set>

namespace UnityContextGen {
namespace Unity {

struct ComponentDependency {
    std::string source_component;
    std::string target_component;
    std::string dependency_type;  // "GetComponent", "RequireComponent", "Reference"
    std::string method_context;   // Method where dependency is used
    size_t line_number;
};

struct ComponentGraph {
    std::map<std::string, std::vector<std::string>> dependencies;
    std::map<std::string, std::vector<std::string>> dependents;
    std::vector<ComponentDependency> edges;
};

class ComponentDependencyAnalyzer {
public:
    ComponentDependencyAnalyzer();
    ~ComponentDependencyAnalyzer() = default;

    void addMonoBehaviour(const MonoBehaviourInfo& mb_info);
    void analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours);
    
    ComponentGraph buildDependencyGraph() const;
    std::vector<ComponentDependency> getDependencies() const;
    
    std::vector<std::string> getComponentDependencies(const std::string& component_name) const;
    std::vector<std::string> getComponentDependents(const std::string& component_name) const;
    
    bool hasCyclicDependencies() const;
    std::vector<std::vector<std::string>> findCyclicDependencies() const;
    
    std::vector<std::string> getTopologicalOrder() const;
    std::map<std::string, int> calculateComponentComplexity() const;

private:
    std::vector<ComponentDependency> m_dependencies;
    std::map<std::string, MonoBehaviourInfo> m_components;
    
    void extractDependenciesFromComponent(const MonoBehaviourInfo& mb_info);
    void analyzeCrossComponentReferences();
    
    bool hasCyclicDependenciesRecursive(const std::string& component, 
                                       std::set<std::string>& visiting,
                                       std::set<std::string>& visited,
                                       const ComponentGraph& graph) const;
    
    void topologicalSortRecursive(const std::string& component,
                                 std::set<std::string>& visited,
                                 std::vector<std::string>& result,
                                 const ComponentGraph& graph) const;
};

} // namespace Unity
} // namespace UnityContextGen