#include "ComponentDependencyAnalyzer.h"
#include <algorithm>
#include <regex>

namespace UnityContextGen {
namespace Unity {

ComponentDependencyAnalyzer::ComponentDependencyAnalyzer() {
}

void ComponentDependencyAnalyzer::addMonoBehaviour(const MonoBehaviourInfo& mb_info) {
    m_components[mb_info.class_name] = mb_info;
    extractDependenciesFromComponent(mb_info);
}

void ComponentDependencyAnalyzer::analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours) {
    m_dependencies.clear();
    m_components.clear();
    
    for (const auto& mb_info : monobehaviours) {
        addMonoBehaviour(mb_info);
    }
    
    analyzeCrossComponentReferences();
}

void ComponentDependencyAnalyzer::extractDependenciesFromComponent(const MonoBehaviourInfo& mb_info) {
    for (const auto& component_dep : mb_info.component_dependencies) {
        ComponentDependency dep;
        dep.source_component = mb_info.class_name;
        dep.target_component = component_dep;
        dep.dependency_type = "GetComponent";
        dep.method_context = "Runtime";
        dep.line_number = 0;
        
        m_dependencies.push_back(dep);
    }
    
    for (const auto& attr : mb_info.attributes) {
        if (attr.find("RequireComponent") != std::string::npos) {
            std::regex require_regex(R"(RequireComponent\(typeof\((\w+)\)\))");
            std::smatch match;
            if (std::regex_search(attr, match, require_regex)) {
                ComponentDependency dep;
                dep.source_component = mb_info.class_name;
                dep.target_component = match[1].str();
                dep.dependency_type = "RequireComponent";
                dep.method_context = "Declaration";
                dep.line_number = mb_info.start_line;
                
                m_dependencies.push_back(dep);
            }
        }
    }
}

void ComponentDependencyAnalyzer::analyzeCrossComponentReferences() {
    for (auto& comp_pair : m_components) {
        const auto& mb_info = comp_pair.second;
        
        for (const auto& field : mb_info.serialized_fields) {
            size_t colon_pos = field.find(" : ");
            if (colon_pos != std::string::npos) {
                std::string field_type = field.substr(colon_pos + 3);
                
                if (m_components.count(field_type) > 0) {
                    ComponentDependency dep;
                    dep.source_component = mb_info.class_name;
                    dep.target_component = field_type;
                    dep.dependency_type = "Reference";
                    dep.method_context = "Field";
                    dep.line_number = 0;
                    
                    m_dependencies.push_back(dep);
                }
            }
        }
    }
}

ComponentGraph ComponentDependencyAnalyzer::buildDependencyGraph() const {
    ComponentGraph graph;
    
    for (const auto& dep : m_dependencies) {
        graph.dependencies[dep.source_component].push_back(dep.target_component);
        graph.dependents[dep.target_component].push_back(dep.source_component);
        graph.edges.push_back(dep);
    }
    
    for (auto& pair : graph.dependencies) {
        std::sort(pair.second.begin(), pair.second.end());
        pair.second.erase(std::unique(pair.second.begin(), pair.second.end()), pair.second.end());
    }
    
    for (auto& pair : graph.dependents) {
        std::sort(pair.second.begin(), pair.second.end());
        pair.second.erase(std::unique(pair.second.begin(), pair.second.end()), pair.second.end());
    }
    
    return graph;
}

std::vector<ComponentDependency> ComponentDependencyAnalyzer::getDependencies() const {
    return m_dependencies;
}

std::vector<std::string> ComponentDependencyAnalyzer::getComponentDependencies(const std::string& component_name) const {
    std::vector<std::string> dependencies;
    
    for (const auto& dep : m_dependencies) {
        if (dep.source_component == component_name) {
            dependencies.push_back(dep.target_component);
        }
    }
    
    std::sort(dependencies.begin(), dependencies.end());
    dependencies.erase(std::unique(dependencies.begin(), dependencies.end()), dependencies.end());
    
    return dependencies;
}

std::vector<std::string> ComponentDependencyAnalyzer::getComponentDependents(const std::string& component_name) const {
    std::vector<std::string> dependents;
    
    for (const auto& dep : m_dependencies) {
        if (dep.target_component == component_name) {
            dependents.push_back(dep.source_component);
        }
    }
    
    std::sort(dependents.begin(), dependents.end());
    dependents.erase(std::unique(dependents.begin(), dependents.end()), dependents.end());
    
    return dependents;
}

bool ComponentDependencyAnalyzer::hasCyclicDependencies() const {
    ComponentGraph graph = buildDependencyGraph();
    std::set<std::string> visited;
    std::set<std::string> visiting;
    
    for (const auto& comp_pair : m_components) {
        const std::string& component = comp_pair.first;
        if (visited.find(component) == visited.end()) {
            if (hasCyclicDependenciesRecursive(component, visiting, visited, graph)) {
                return true;
            }
        }
    }
    
    return false;
}

bool ComponentDependencyAnalyzer::hasCyclicDependenciesRecursive(const std::string& component,
                                                                std::set<std::string>& visiting,
                                                                std::set<std::string>& visited,
                                                                const ComponentGraph& graph) const {
    visiting.insert(component);
    
    auto deps_it = graph.dependencies.find(component);
    if (deps_it != graph.dependencies.end()) {
        for (const auto& dep : deps_it->second) {
            if (visiting.find(dep) != visiting.end()) {
                return true;
            }
            
            if (visited.find(dep) == visited.end()) {
                if (hasCyclicDependenciesRecursive(dep, visiting, visited, graph)) {
                    return true;
                }
            }
        }
    }
    
    visiting.erase(component);
    visited.insert(component);
    
    return false;
}

std::vector<std::vector<std::string>> ComponentDependencyAnalyzer::findCyclicDependencies() const {
    std::vector<std::vector<std::string>> cycles;
    return cycles;
}

std::vector<std::string> ComponentDependencyAnalyzer::getTopologicalOrder() const {
    ComponentGraph graph = buildDependencyGraph();
    std::vector<std::string> result;
    std::set<std::string> visited;
    
    for (const auto& comp_pair : m_components) {
        const std::string& component = comp_pair.first;
        if (visited.find(component) == visited.end()) {
            topologicalSortRecursive(component, visited, result, graph);
        }
    }
    
    std::reverse(result.begin(), result.end());
    return result;
}

void ComponentDependencyAnalyzer::topologicalSortRecursive(const std::string& component,
                                                          std::set<std::string>& visited,
                                                          std::vector<std::string>& result,
                                                          const ComponentGraph& graph) const {
    visited.insert(component);
    
    auto deps_it = graph.dependencies.find(component);
    if (deps_it != graph.dependencies.end()) {
        for (const auto& dep : deps_it->second) {
            if (visited.find(dep) == visited.end()) {
                topologicalSortRecursive(dep, visited, result, graph);
            }
        }
    }
    
    result.push_back(component);
}

std::map<std::string, int> ComponentDependencyAnalyzer::calculateComponentComplexity() const {
    std::map<std::string, int> complexity;
    
    for (const auto& comp_pair : m_components) {
        const std::string& comp_name = comp_pair.first;
        const MonoBehaviourInfo& mb_info = comp_pair.second;
        
        int score = 0;
        score += mb_info.unity_methods.size() * 2;
        score += mb_info.custom_methods.size();
        score += mb_info.component_dependencies.size() * 3;
        score += mb_info.serialized_fields.size();
        
        complexity[comp_name] = score;
    }
    
    return complexity;
}

} // namespace Unity
} // namespace UnityContextGen