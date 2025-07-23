#include "ProjectMetadata.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <regex>

namespace UnityContextGen {
namespace Metadata {

ProjectMetadataGenerator::ProjectMetadataGenerator() {
}

void ProjectMetadataGenerator::analyzeProject(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                                             const Unity::ComponentGraph& dependency_graph,
                                             const std::vector<Unity::LifecycleFlow>& lifecycle_flows,
                                             const std::vector<Unity::PatternInstance>& patterns,
                                             const std::vector<std::string>& file_paths) {
    m_project_metadata = ProjectMetadata();
    
    m_component_generator.analyzeProject(monobehaviours, dependency_graph, lifecycle_flows, patterns);
    auto component_metadata = m_component_generator.getAllComponentMetadata();
    
    for (const auto& metadata : component_metadata) {
        m_project_metadata.components[metadata.class_name] = metadata;
    }
    
    m_project_metadata.files_analyzed = file_paths;
    
    std::time_t now = std::time(nullptr);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    m_project_metadata.analysis_timestamp = ss.str();
    
    analyzeProjectStructure(monobehaviours);
    analyzeSystemGroups(monobehaviours, dependency_graph);
    analyzeDependencyStructure(dependency_graph);
    analyzePatternUsage(patterns);
    calculateQualityMetrics(monobehaviours, dependency_graph);
}

void ProjectMetadataGenerator::analyzeProjectStructure(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) {
    m_project_metadata.project_summary.total_components = monobehaviours.size();
    
    int total_dependencies = 0;
    for (const auto& mb_info : monobehaviours) {
        total_dependencies += mb_info.component_dependencies.size();
    }
    m_project_metadata.project_summary.total_dependencies = total_dependencies;
    
    m_project_metadata.project_summary.game_type = inferGameType(monobehaviours);
    m_project_metadata.project_summary.complexity_level = inferComplexityLevel(monobehaviours);
    m_project_metadata.project_summary.key_systems = identifyKeySystems(monobehaviours);
}

void ProjectMetadataGenerator::analyzeSystemGroups(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                                                   const Unity::ComponentGraph& dependency_graph) {
    std::map<std::string, std::vector<std::string>> groups;
    
    for (const auto& mb_info : monobehaviours) {
        std::string system_name = "Unknown";
        
        std::string class_name = mb_info.class_name;
        std::transform(class_name.begin(), class_name.end(), class_name.begin(), ::tolower);
        
        if (class_name.find("player") != std::string::npos) {
            system_name = "Player System";
        } else if (class_name.find("enemy") != std::string::npos || class_name.find("ai") != std::string::npos) {
            system_name = "AI System";
        } else if (class_name.find("ui") != std::string::npos || class_name.find("menu") != std::string::npos) {
            system_name = "UI System";
        } else if (class_name.find("camera") != std::string::npos) {
            system_name = "Camera System";
        } else if (class_name.find("audio") != std::string::npos || class_name.find("sound") != std::string::npos) {
            system_name = "Audio System";
        } else if (class_name.find("physics") != std::string::npos) {
            system_name = "Physics System";
        } else if (class_name.find("manager") != std::string::npos) {
            system_name = "Management System";
        } else if (class_name.find("controller") != std::string::npos) {
            system_name = "Control System";
        } else {
            system_name = "Core System";
        }
        
        groups[system_name].push_back(mb_info.class_name);
    }
    
    m_project_metadata.systems.system_groups = groups;
    
    for (const auto& group : groups) {
        if (group.second.size() >= 3) {
            m_project_metadata.systems.core_systems.push_back(group.first);
        } else {
            m_project_metadata.systems.support_systems.push_back(group.first);
        }
        
        int complexity = 0;
        for (const auto& component : group.second) {
            auto comp_it = m_project_metadata.components.find(component);
            if (comp_it != m_project_metadata.components.end()) {
                complexity += comp_it->second.metrics.complexity_score;
            }
        }
        m_project_metadata.systems.system_complexity[group.first] = complexity;
    }
}

void ProjectMetadataGenerator::analyzeDependencyStructure(const Unity::ComponentGraph& dependency_graph) {
    m_project_metadata.dependencies.dependency_clusters = findDependencyClusters(dependency_graph);
    m_project_metadata.dependencies.high_coupling_components = findHighCouplingComponents(dependency_graph);
    
    for (const auto& comp_pair : m_project_metadata.components) {
        if (comp_pair.second.dependencies.empty() && comp_pair.second.dependents.empty()) {
            m_project_metadata.dependencies.isolated_components.push_back(comp_pair.first);
        }
    }
    
    Unity::ComponentDependencyAnalyzer analyzer;
    for (const auto& comp_pair : m_project_metadata.components) {
        Unity::MonoBehaviourInfo mb_info;
        mb_info.class_name = comp_pair.first;
        mb_info.component_dependencies = comp_pair.second.dependencies;
        analyzer.addMonoBehaviour(mb_info);
    }
    
    m_project_metadata.dependencies.has_circular_dependencies = analyzer.hasCyclicDependencies();
    m_project_metadata.dependencies.topological_order = analyzer.getTopologicalOrder();
}

void ProjectMetadataGenerator::analyzePatternUsage(const std::vector<Unity::PatternInstance>& patterns) {
    std::map<std::string, int> frequency;
    
    for (const auto& pattern : patterns) {
        frequency[pattern.pattern_name]++;
    }
    
    m_project_metadata.patterns.pattern_frequency = frequency;
    
    int max_count = 0;
    for (const auto& pair : frequency) {
        if (pair.second > max_count) {
            max_count = pair.second;
        }
    }
    
    for (const auto& pair : frequency) {
        if (pair.second == max_count && max_count > 0) {
            m_project_metadata.patterns.dominant_patterns.push_back(pair.first);
        }
    }
    
    float consistency = 0.0f;
    if (!patterns.empty()) {
        float total_confidence = 0.0f;
        for (const auto& pattern : patterns) {
            total_confidence += pattern.confidence_score;
        }
        consistency = total_confidence / patterns.size();
    }
    m_project_metadata.patterns.pattern_consistency_score = consistency;
    
    m_project_metadata.project_summary.architecture_pattern = inferArchitecturePattern(patterns);
}

void ProjectMetadataGenerator::calculateQualityMetrics(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                                                      const Unity::ComponentGraph& dependency_graph) {
    m_project_metadata.quality.maintainability_score = calculateMaintainabilityScore(monobehaviours);
    m_project_metadata.quality.testability_score = calculateTestabilityScore(dependency_graph);
    m_project_metadata.quality.performance_score = calculatePerformanceScore(monobehaviours);
    
    std::vector<Unity::PatternInstance> patterns;
    m_project_metadata.quality.architecture_score = calculateArchitectureScore(patterns);
    
    m_project_metadata.quality.improvement_suggestions = generateImprovementSuggestions();
}

std::string ProjectMetadataGenerator::inferArchitecturePattern(const std::vector<Unity::PatternInstance>& patterns) const {
    std::map<std::string, int> architecture_votes;
    
    for (const auto& pattern : patterns) {
        if (pattern.pattern_name.find("MVC") != std::string::npos) {
            architecture_votes["Model-View-Controller"]++;
        } else if (pattern.pattern_name.find("ECS") != std::string::npos) {
            architecture_votes["Entity-Component-System"]++;
        } else if (pattern.pattern_name.find("Component") != std::string::npos) {
            architecture_votes["Component-based"]++;
        } else if (pattern.pattern_name.find("Observer") != std::string::npos) {
            architecture_votes["Event-driven"]++;
        } else if (pattern.pattern_name.find("State") != std::string::npos) {
            architecture_votes["State-based"]++;
        }
    }
    
    std::string dominant_architecture = "Component-based";
    int max_votes = 0;
    
    for (const auto& vote : architecture_votes) {
        if (vote.second > max_votes) {
            max_votes = vote.second;
            dominant_architecture = vote.first;
        }
    }
    
    return dominant_architecture;
}

std::string ProjectMetadataGenerator::inferGameType(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const {
    std::map<std::string, int> type_indicators;
    
    for (const auto& mb_info : monobehaviours) {
        std::string class_name = mb_info.class_name;
        std::transform(class_name.begin(), class_name.end(), class_name.begin(), ::tolower);
        
        if (class_name.find("platformer") != std::string::npos || 
            class_name.find("jump") != std::string::npos) {
            type_indicators["Platformer"]++;
        } else if (class_name.find("fps") != std::string::npos || 
                   class_name.find("shooter") != std::string::npos) {
            type_indicators["First-Person Shooter"]++;
        } else if (class_name.find("rpg") != std::string::npos || 
                   class_name.find("inventory") != std::string::npos) {
            type_indicators["RPG"]++;
        } else if (class_name.find("puzzle") != std::string::npos) {
            type_indicators["Puzzle"]++;
        } else if (class_name.find("racing") != std::string::npos || 
                   class_name.find("car") != std::string::npos) {
            type_indicators["Racing"]++;
        } else if (class_name.find("strategy") != std::string::npos || 
                   class_name.find("rts") != std::string::npos) {
            type_indicators["Strategy"]++;
        }
    }
    
    if (type_indicators.empty()) {
        return "Generic Game";
    }
    
    std::string game_type = "Generic Game";
    int max_count = 0;
    
    for (const auto& indicator : type_indicators) {
        if (indicator.second > max_count) {
            max_count = indicator.second;
            game_type = indicator.first;
        }
    }
    
    return game_type;
}

std::string ProjectMetadataGenerator::inferComplexityLevel(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const {
    int total_complexity = 0;
    
    for (const auto& mb_info : monobehaviours) {
        total_complexity += mb_info.unity_methods.size() * 2;
        total_complexity += mb_info.custom_methods.size();
        total_complexity += mb_info.component_dependencies.size() * 3;
    }
    
    float avg_complexity = static_cast<float>(total_complexity) / monobehaviours.size();
    
    if (avg_complexity < 10) {
        return "Simple";
    } else if (avg_complexity < 25) {
        return "Moderate";
    } else if (avg_complexity < 50) {
        return "Complex";
    } else {
        return "Very Complex";
    }
}

std::vector<std::string> ProjectMetadataGenerator::identifyKeySystems(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const {
    std::map<std::string, int> system_importance;
    
    for (const auto& mb_info : monobehaviours) {
        std::string class_name = mb_info.class_name;
        std::transform(class_name.begin(), class_name.end(), class_name.begin(), ::tolower);
        
        if (class_name.find("player") != std::string::npos) {
            system_importance["Player Control"]++;
        } else if (class_name.find("enemy") != std::string::npos || class_name.find("ai") != std::string::npos) {
            system_importance["Enemy AI"]++;
        } else if (class_name.find("physics") != std::string::npos) {
            system_importance["Physics Interaction"]++;
        } else if (class_name.find("ui") != std::string::npos) {
            system_importance["User Interface"]++;
        } else if (class_name.find("audio") != std::string::npos) {
            system_importance["Audio System"]++;
        } else if (class_name.find("camera") != std::string::npos) {
            system_importance["Camera Control"]++;
        }
    }
    
    std::vector<std::string> key_systems;
    for (const auto& system : system_importance) {
        if (system.second > 0) {
            key_systems.push_back(system.first);
        }
    }
    
    return key_systems;
}

std::vector<std::string> ProjectMetadataGenerator::findDependencyClusters(const Unity::ComponentGraph& dependency_graph) const {
    std::vector<std::string> clusters;
    
    for (const auto& dep_pair : dependency_graph.dependencies) {
        if (dep_pair.second.size() >= 3) {
            std::string cluster_desc = dep_pair.first + " cluster (" + std::to_string(dep_pair.second.size()) + " dependencies)";
            clusters.push_back(cluster_desc);
        }
    }
    
    return clusters;
}

std::vector<std::string> ProjectMetadataGenerator::findHighCouplingComponents(const Unity::ComponentGraph& dependency_graph) const {
    std::vector<std::string> high_coupling;
    
    for (const auto& dep_pair : dependency_graph.dependencies) {
        if (dep_pair.second.size() >= 5) {
            high_coupling.push_back(dep_pair.first);
        }
    }
    
    for (const auto& dep_pair : dependency_graph.dependents) {
        if (dep_pair.second.size() >= 5) {
            if (std::find(high_coupling.begin(), high_coupling.end(), dep_pair.first) == high_coupling.end()) {
                high_coupling.push_back(dep_pair.first);
            }
        }
    }
    
    return high_coupling;
}

float ProjectMetadataGenerator::calculateMaintainabilityScore(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const {
    float score = 100.0f;
    
    for (const auto& mb_info : monobehaviours) {
        if (mb_info.unity_methods.size() > 10) {
            score -= 5.0f;
        }
        if (mb_info.component_dependencies.size() > 5) {
            score -= 10.0f;
        }
        if (mb_info.custom_methods.size() > 20) {
            score -= 5.0f;
        }
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ProjectMetadataGenerator::calculateTestabilityScore(const Unity::ComponentGraph& dependency_graph) const {
    float score = 100.0f;
    
    int high_coupling_count = 0;
    for (const auto& dep_pair : dependency_graph.dependencies) {
        if (dep_pair.second.size() > 3) {
            high_coupling_count++;
        }
    }
    
    score -= high_coupling_count * 10.0f;
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ProjectMetadataGenerator::calculatePerformanceScore(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const {
    float score = 100.0f;
    
    int update_methods = 0;
    for (const auto& mb_info : monobehaviours) {
        if (std::find(mb_info.unity_methods.begin(), mb_info.unity_methods.end(), "Update") 
            != mb_info.unity_methods.end()) {
            update_methods++;
        }
    }
    
    if (update_methods > 10) {
        score -= (update_methods - 10) * 5.0f;
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ProjectMetadataGenerator::calculateArchitectureScore(const std::vector<Unity::PatternInstance>& patterns) const {
    float score = 50.0f;
    
    for (const auto& pattern : patterns) {
        score += pattern.confidence_score * 10.0f;
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

std::vector<std::string> ProjectMetadataGenerator::generateImprovementSuggestions() const {
    std::vector<std::string> suggestions;
    
    if (m_project_metadata.quality.maintainability_score < 70) {
        suggestions.push_back("Consider reducing component complexity by breaking down large classes");
    }
    
    if (m_project_metadata.quality.testability_score < 70) {
        suggestions.push_back("Reduce coupling between components for better testability");
    }
    
    if (m_project_metadata.quality.performance_score < 70) {
        suggestions.push_back("Optimize Update methods and consider object pooling");
    }
    
    if (m_project_metadata.dependencies.has_circular_dependencies) {
        suggestions.push_back("Resolve circular dependencies to improve architecture");
    }
    
    if (!m_project_metadata.dependencies.high_coupling_components.empty()) {
        suggestions.push_back("Refactor high-coupling components using dependency injection");
    }
    
    return suggestions;
}

ProjectMetadata ProjectMetadataGenerator::getProjectMetadata() const {
    return m_project_metadata;
}

nlohmann::json ProjectMetadataGenerator::exportToJSON() const {
    nlohmann::json json_output;
    
    json_output["project_summary"] = {
        {"architecture_pattern", m_project_metadata.project_summary.architecture_pattern},
        {"key_systems", m_project_metadata.project_summary.key_systems},
        {"data_flow", m_project_metadata.project_summary.data_flow},
        {"game_type", m_project_metadata.project_summary.game_type},
        {"complexity_level", m_project_metadata.project_summary.complexity_level},
        {"total_components", m_project_metadata.project_summary.total_components},
        {"total_dependencies", m_project_metadata.project_summary.total_dependencies}
    };
    
    json_output["systems"] = {
        {"system_groups", m_project_metadata.systems.system_groups},
        {"core_systems", m_project_metadata.systems.core_systems},
        {"support_systems", m_project_metadata.systems.support_systems},
        {"system_complexity", m_project_metadata.systems.system_complexity}
    };
    
    json_output["dependencies"] = {
        {"dependency_clusters", m_project_metadata.dependencies.dependency_clusters},
        {"high_coupling_components", m_project_metadata.dependencies.high_coupling_components},
        {"isolated_components", m_project_metadata.dependencies.isolated_components},
        {"has_circular_dependencies", m_project_metadata.dependencies.has_circular_dependencies},
        {"topological_order", m_project_metadata.dependencies.topological_order}
    };
    
    json_output["patterns"] = {
        {"pattern_frequency", m_project_metadata.patterns.pattern_frequency},
        {"dominant_patterns", m_project_metadata.patterns.dominant_patterns},
        {"pattern_consistency_score", m_project_metadata.patterns.pattern_consistency_score},
        {"recommended_patterns", m_project_metadata.patterns.recommended_patterns}
    };
    
    json_output["quality"] = {
        {"maintainability_score", m_project_metadata.quality.maintainability_score},
        {"testability_score", m_project_metadata.quality.testability_score},
        {"performance_score", m_project_metadata.quality.performance_score},
        {"architecture_score", m_project_metadata.quality.architecture_score},
        {"improvement_suggestions", m_project_metadata.quality.improvement_suggestions}
    };
    
    json_output["components"] = m_component_generator.exportToJSON()["components"];
    
    json_output["metadata"] = {
        {"files_analyzed", m_project_metadata.files_analyzed},
        {"analysis_timestamp", m_project_metadata.analysis_timestamp}
    };
    
    return json_output;
}

nlohmann::json ProjectMetadataGenerator::exportSummaryToJSON() const {
    nlohmann::json summary;
    
    summary["project_summary"] = {
        {"architecture_pattern", m_project_metadata.project_summary.architecture_pattern},
        {"key_systems", m_project_metadata.project_summary.key_systems},
        {"game_type", m_project_metadata.project_summary.game_type},
        {"complexity_level", m_project_metadata.project_summary.complexity_level},
        {"total_components", m_project_metadata.project_summary.total_components}
    };
    
    summary["quality_scores"] = {
        {"maintainability", m_project_metadata.quality.maintainability_score},
        {"testability", m_project_metadata.quality.testability_score},
        {"performance", m_project_metadata.quality.performance_score},
        {"architecture", m_project_metadata.quality.architecture_score}
    };
    
    summary["dominant_patterns"] = m_project_metadata.patterns.dominant_patterns;
    summary["improvement_suggestions"] = m_project_metadata.quality.improvement_suggestions;
    
    return summary;
}

std::string ProjectMetadataGenerator::generateProjectSummary() const {
    std::stringstream summary;
    
    summary << "Unity Project Analysis Summary\n";
    summary << "==============================\n\n";
    
    summary << "Project Type: " << m_project_metadata.project_summary.game_type << "\n";
    summary << "Architecture: " << m_project_metadata.project_summary.architecture_pattern << "\n";
    summary << "Complexity: " << m_project_metadata.project_summary.complexity_level << "\n";
    summary << "Components: " << m_project_metadata.project_summary.total_components << "\n";
    summary << "Dependencies: " << m_project_metadata.project_summary.total_dependencies << "\n\n";
    
    summary << "Key Systems:\n";
    for (const auto& system : m_project_metadata.project_summary.key_systems) {
        summary << "  - " << system << "\n";
    }
    
    summary << "\nQuality Metrics:\n";
    summary << "  Maintainability: " << m_project_metadata.quality.maintainability_score << "%\n";
    summary << "  Testability: " << m_project_metadata.quality.testability_score << "%\n";
    summary << "  Performance: " << m_project_metadata.quality.performance_score << "%\n";
    summary << "  Architecture: " << m_project_metadata.quality.architecture_score << "%\n";
    
    if (!m_project_metadata.patterns.dominant_patterns.empty()) {
        summary << "\nDominant Patterns:\n";
        for (const auto& pattern : m_project_metadata.patterns.dominant_patterns) {
            summary << "  - " << pattern << "\n";
        }
    }
    
    return summary.str();
}

std::string ProjectMetadataGenerator::generateArchitectureReport() const {
    std::stringstream report;
    
    report << "Architecture Analysis Report\n";
    report << "===========================\n\n";
    
    report << "Overall Architecture: " << m_project_metadata.project_summary.architecture_pattern << "\n\n";
    
    report << "System Groups:\n";
    for (const auto& group : m_project_metadata.systems.system_groups) {
        report << "  " << group.first << " (" << group.second.size() << " components)\n";
        for (const auto& component : group.second) {
            report << "    - " << component << "\n";
        }
    }
    
    report << "\nDependency Analysis:\n";
    if (m_project_metadata.dependencies.has_circular_dependencies) {
        report << "  WARNING: Circular dependencies detected!\n";
    } else {
        report << "  No circular dependencies found.\n";
    }
    
    if (!m_project_metadata.dependencies.high_coupling_components.empty()) {
        report << "  High coupling components:\n";
        for (const auto& component : m_project_metadata.dependencies.high_coupling_components) {
            report << "    - " << component << "\n";
        }
    }
    
    return report.str();
}

std::string ProjectMetadataGenerator::generateRecommendations() const {
    std::stringstream recommendations;
    
    recommendations << "Improvement Recommendations\n";
    recommendations << "==========================\n\n";
    
    for (const auto& suggestion : m_project_metadata.quality.improvement_suggestions) {
        recommendations << "• " << suggestion << "\n";
    }
    
    if (m_project_metadata.quality.improvement_suggestions.empty()) {
        recommendations << "No specific improvements identified. Project architecture looks good!\n";
    }
    
    return recommendations.str();
}

} // namespace Metadata
} // namespace UnityContextGen