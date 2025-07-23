#pragma once

#include "ComponentMetadata.h"
#include "../UnityAnalyzer/UnityPatternDetector.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

namespace UnityContextGen {
namespace Metadata {

struct ProjectMetadata {
    struct ProjectSummary {
        std::string architecture_pattern;
        std::vector<std::string> key_systems;
        std::string data_flow;
        std::string game_type;
        std::string complexity_level;
        int total_components;
        int total_dependencies;
    } project_summary;
    
    struct SystemAnalysis {
        std::map<std::string, std::vector<std::string>> system_groups;
        std::vector<std::string> core_systems;
        std::vector<std::string> support_systems;
        std::map<std::string, int> system_complexity;
    } systems;
    
    struct DependencyAnalysis {
        std::vector<std::string> dependency_clusters;
        std::vector<std::string> high_coupling_components;
        std::vector<std::string> isolated_components;
        bool has_circular_dependencies;
        std::vector<std::string> topological_order;
    } dependencies;
    
    struct PatternAnalysis {
        std::map<std::string, int> pattern_frequency;
        std::vector<std::string> dominant_patterns;
        float pattern_consistency_score;
        std::vector<std::string> recommended_patterns;
    } patterns;
    
    struct QualityMetrics {
        float maintainability_score;
        float testability_score;
        float performance_score;
        float architecture_score;
        std::vector<std::string> improvement_suggestions;
    } quality;
    
    std::map<std::string, ComponentMetadata> components;
    std::vector<std::string> files_analyzed;
    std::string analysis_timestamp;
};

class ProjectMetadataGenerator {
public:
    ProjectMetadataGenerator();
    ~ProjectMetadataGenerator() = default;

    void analyzeProject(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                       const Unity::ComponentGraph& dependency_graph,
                       const std::vector<Unity::LifecycleFlow>& lifecycle_flows,
                       const std::vector<Unity::PatternInstance>& patterns,
                       const std::vector<std::string>& file_paths);
    
    ProjectMetadata getProjectMetadata() const;
    nlohmann::json exportToJSON() const;
    nlohmann::json exportSummaryToJSON() const;
    
    std::string generateProjectSummary() const;
    std::string generateArchitectureReport() const;
    std::string generateRecommendations() const;

private:
    ProjectMetadata m_project_metadata;
    ComponentMetadataGenerator m_component_generator;
    
    void analyzeProjectStructure(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours);
    void analyzeSystemGroups(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                            const Unity::ComponentGraph& dependency_graph);
    void analyzeDependencyStructure(const Unity::ComponentGraph& dependency_graph);
    void analyzePatternUsage(const std::vector<Unity::PatternInstance>& patterns);
    void calculateQualityMetrics(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                                const Unity::ComponentGraph& dependency_graph);
    
    std::string inferArchitecturePattern(const std::vector<Unity::PatternInstance>& patterns) const;
    std::string inferGameType(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
    std::string inferComplexityLevel(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
    std::string generateDataFlowDescription(const std::vector<Unity::LifecycleFlow>& lifecycle_flows) const;
    
    std::vector<std::string> identifyKeySystems(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
    std::vector<std::string> findDependencyClusters(const Unity::ComponentGraph& dependency_graph) const;
    std::vector<std::string> findHighCouplingComponents(const Unity::ComponentGraph& dependency_graph) const;
    
    float calculateMaintainabilityScore(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
    float calculateTestabilityScore(const Unity::ComponentGraph& dependency_graph) const;
    float calculatePerformanceScore(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
    float calculateArchitectureScore(const std::vector<Unity::PatternInstance>& patterns) const;
    
    std::vector<std::string> generateImprovementSuggestions() const;
    std::vector<std::string> recommendPatterns(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours) const;
};

} // namespace Metadata
} // namespace UnityContextGen