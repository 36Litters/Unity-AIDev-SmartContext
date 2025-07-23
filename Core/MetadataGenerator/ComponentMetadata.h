#pragma once

#include "../UnityAnalyzer/MonoBehaviourAnalyzer.h"
#include "../UnityAnalyzer/ComponentDependencyAnalyzer.h"
#include "../UnityAnalyzer/LifecycleAnalyzer.h"
#include "../UnityAnalyzer/UnityPatternDetector.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

namespace UnityContextGen {
namespace Metadata {

struct ComponentMetadata {
    std::string class_name;
    std::string file_path;
    std::string purpose;
    std::vector<std::string> dependencies;
    std::vector<std::string> dependents;
    
    struct LifecycleInfo {
        std::vector<std::string> methods;
        std::map<std::string, std::string> method_purposes;
        std::vector<std::string> execution_order;
        std::vector<std::string> data_flow;
    } lifecycle;
    
    struct ResponsibilityBlocks {
        std::map<std::string, std::string> blocks;
        std::vector<std::string> block_order;
    } responsibility_blocks;
    
    std::vector<std::string> serialized_fields;
    std::vector<std::string> attributes;
    std::vector<std::string> design_patterns;
    
    struct MetricsInfo {
        int complexity_score;
        int dependency_count;
        int method_count;
        int unity_method_count;
        float pattern_confidence;
    } metrics;
    
    size_t start_line;
    size_t end_line;
};

class ComponentMetadataGenerator {
public:
    ComponentMetadataGenerator();
    ~ComponentMetadataGenerator() = default;

    void analyzeComponent(const Unity::MonoBehaviourInfo& mb_info,
                         const Unity::ComponentGraph& dependency_graph,
                         const Unity::LifecycleFlow& lifecycle_flow,
                         const std::vector<Unity::PatternInstance>& patterns);
    
    void analyzeProject(const std::vector<Unity::MonoBehaviourInfo>& monobehaviours,
                       const Unity::ComponentGraph& dependency_graph,
                       const std::vector<Unity::LifecycleFlow>& lifecycle_flows,
                       const std::vector<Unity::PatternInstance>& patterns);
    
    ComponentMetadata getComponentMetadata(const std::string& component_name) const;
    std::vector<ComponentMetadata> getAllComponentMetadata() const;
    
    nlohmann::json exportToJSON() const;
    nlohmann::json exportComponentToJSON(const std::string& component_name) const;
    
    void generateResponsibilityBlocks(ComponentMetadata& metadata,
                                     const Unity::MonoBehaviourInfo& mb_info) const;
    
    std::string inferComponentPurpose(const Unity::MonoBehaviourInfo& mb_info,
                                     const std::vector<Unity::PatternInstance>& patterns) const;

private:
    std::map<std::string, ComponentMetadata> m_component_metadata;
    
    void populateLifecycleInfo(ComponentMetadata& metadata,
                              const Unity::LifecycleFlow& lifecycle_flow) const;
    
    void populatePatternInfo(ComponentMetadata& metadata,
                            const std::vector<Unity::PatternInstance>& patterns) const;
    
    void calculateMetrics(ComponentMetadata& metadata,
                         const Unity::MonoBehaviourInfo& mb_info,
                         const Unity::ComponentGraph& dependency_graph) const;
    
    std::vector<std::string> extractResponsibilityKeywords(const Unity::MonoBehaviourInfo& mb_info) const;
    std::string categorizeResponsibility(const std::string& method_name) const;
};

} // namespace Metadata
} // namespace UnityContextGen