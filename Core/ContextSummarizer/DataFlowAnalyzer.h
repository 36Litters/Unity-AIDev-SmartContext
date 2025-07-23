#pragma once

#include "../MetadataGenerator/ProjectMetadata.h"
#include <string>
#include <vector>
#include <map>
#include <set>

namespace UnityContextGen {
namespace Context {

enum class DataFlowDirection {
    Input,
    Processing,
    Output,
    Bidirectional,
    Internal
};

struct DataFlowNode {
    std::string component_name;
    std::string data_type;
    DataFlowDirection direction;
    std::vector<std::string> connected_components;
    std::string responsibility;
    int priority_level;
};

struct DataFlowPath {
    std::vector<std::string> path_components;
    std::string data_description;
    std::string flow_type; // "Input → Logic → Output", "Event-driven", etc.
    float criticality_score;
    std::vector<std::string> bottlenecks;
};

struct DataFlowInsights {
    std::vector<DataFlowPath> primary_flows;
    std::vector<DataFlowPath> secondary_flows;
    std::vector<std::string> data_hotspots;
    std::vector<std::string> isolated_components;
    std::string dominant_flow_pattern;
    float flow_efficiency_score;
};

class DataFlowAnalyzer {
public:
    DataFlowAnalyzer();
    ~DataFlowAnalyzer() = default;

    DataFlowInsights analyzeDataFlow(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<DataFlowNode> buildDataFlowGraph(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<DataFlowPath> identifyPrimaryFlows(const std::vector<DataFlowNode>& nodes) const;
    
    std::string generateDataFlowSummary(const Metadata::ProjectMetadata& project_metadata) const;
    std::string generateFlowOptimizationReport(const DataFlowInsights& insights) const;
    
    std::vector<std::string> identifyBottlenecks(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> findDataHotspots(const std::vector<DataFlowNode>& nodes) const;
    
    float calculateFlowEfficiency(const std::vector<DataFlowPath>& flows) const;

private:
    DataFlowDirection inferDataDirection(const std::string& component_name,
                                       const Metadata::ComponentMetadata& metadata) const;
    
    std::string inferDataType(const Metadata::ComponentMetadata& metadata) const;
    int calculatePriority(const Metadata::ComponentMetadata& metadata) const;
    
    std::vector<DataFlowPath> traceDataPaths(const std::vector<DataFlowNode>& nodes) const;
    std::string identifyFlowType(const DataFlowPath& path) const;
    float calculateCriticality(const DataFlowPath& path, 
                              const Metadata::ProjectMetadata& project_metadata) const;
    
    std::string determineDominantPattern(const std::vector<DataFlowPath>& flows) const;
    
    bool isInputComponent(const std::string& component_name,
                         const Metadata::ComponentMetadata& metadata) const;
    bool isProcessingComponent(const std::string& component_name,
                             const Metadata::ComponentMetadata& metadata) const;
    bool isOutputComponent(const std::string& component_name,
                          const Metadata::ComponentMetadata& metadata) const;
    
    std::vector<std::string> findPathBottlenecks(const DataFlowPath& path,
                                               const Metadata::ProjectMetadata& project_metadata) const;
};

} // namespace Context
} // namespace UnityContextGen