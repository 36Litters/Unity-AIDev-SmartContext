#include "DataFlowAnalyzer.h"
#include <algorithm>
#include <numeric>
#include <sstream>

namespace UnityContextGen {
namespace Context {

DataFlowAnalyzer::DataFlowAnalyzer() {
}

DataFlowInsights DataFlowAnalyzer::analyzeDataFlow(const Metadata::ProjectMetadata& project_metadata) const {
    DataFlowInsights insights;
    
    auto nodes = buildDataFlowGraph(project_metadata);
    auto all_flows = traceDataPaths(nodes);
    
    std::sort(all_flows.begin(), all_flows.end(),
              [](const DataFlowPath& a, const DataFlowPath& b) {
                  return a.criticality_score > b.criticality_score;
              });
    
    size_t primary_count = std::min(static_cast<size_t>(3), all_flows.size());
    insights.primary_flows.assign(all_flows.begin(), all_flows.begin() + primary_count);
    
    if (all_flows.size() > primary_count) {
        insights.secondary_flows.assign(all_flows.begin() + primary_count, all_flows.end());
    }
    
    insights.data_hotspots = findDataHotspots(nodes);
    
    std::set<std::string> connected_components;
    for (const auto& flow : all_flows) {
        for (const auto& comp : flow.path_components) {
            connected_components.insert(comp);
        }
    }
    
    for (const auto& comp_pair : project_metadata.components) {
        if (connected_components.find(comp_pair.first) == connected_components.end()) {
            insights.isolated_components.push_back(comp_pair.first);
        }
    }
    
    insights.dominant_flow_pattern = determineDominantPattern(all_flows);
    insights.flow_efficiency_score = calculateFlowEfficiency(all_flows);
    
    return insights;
}

std::vector<DataFlowNode> DataFlowAnalyzer::buildDataFlowGraph(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<DataFlowNode> nodes;
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp_name = comp_pair.first;
        const auto& comp_metadata = comp_pair.second;
        
        DataFlowNode node;
        node.component_name = comp_name;
        node.data_type = inferDataType(comp_metadata);
        node.direction = inferDataDirection(comp_name, comp_metadata);
        node.connected_components = comp_metadata.dependencies;
        node.responsibility = comp_metadata.purpose;
        node.priority_level = calculatePriority(comp_metadata);
        
        nodes.push_back(node);
    }
    
    return nodes;
}

std::vector<DataFlowPath> DataFlowAnalyzer::identifyPrimaryFlows(const std::vector<DataFlowNode>& nodes) const {
    auto flows = traceDataPaths(nodes);
    
    std::sort(flows.begin(), flows.end(),
              [](const DataFlowPath& a, const DataFlowPath& b) {
                  return a.criticality_score > b.criticality_score;
              });
    
    size_t primary_count = std::min(static_cast<size_t>(5), flows.size());
    return std::vector<DataFlowPath>(flows.begin(), flows.begin() + primary_count);
}

DataFlowDirection DataFlowAnalyzer::inferDataDirection(const std::string& component_name,
                                                      const Metadata::ComponentMetadata& metadata) const {
    std::string comp_lower = component_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    if (isInputComponent(component_name, metadata)) {
        return DataFlowDirection::Input;
    } else if (isOutputComponent(component_name, metadata)) {
        return DataFlowDirection::Output;
    } else if (isProcessingComponent(component_name, metadata)) {
        return DataFlowDirection::Processing;
    } else if (!metadata.dependencies.empty() && !metadata.dependents.empty()) {
        return DataFlowDirection::Bidirectional;
    } else {
        return DataFlowDirection::Internal;
    }
}

std::string DataFlowAnalyzer::inferDataType(const Metadata::ComponentMetadata& metadata) const {
    std::string comp_lower = metadata.class_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    if (comp_lower.find("input") != std::string::npos || comp_lower.find("controller") != std::string::npos) {
        return "Input Data";
    } else if (comp_lower.find("ui") != std::string::npos || comp_lower.find("display") != std::string::npos) {
        return "UI Data";
    } else if (comp_lower.find("physics") != std::string::npos || comp_lower.find("movement") != std::string::npos) {
        return "Physics Data";
    } else if (comp_lower.find("audio") != std::string::npos || comp_lower.find("sound") != std::string::npos) {
        return "Audio Data";
    } else if (comp_lower.find("ai") != std::string::npos || comp_lower.find("enemy") != std::string::npos) {
        return "AI Data";
    } else if (comp_lower.find("camera") != std::string::npos) {
        return "Camera Data";
    } else if (comp_lower.find("manager") != std::string::npos) {
        return "Management Data";
    } else {
        return "Game Data";
    }
}

int DataFlowAnalyzer::calculatePriority(const Metadata::ComponentMetadata& metadata) const {
    int priority = 1;
    
    if (metadata.dependents.size() > 3) {
        priority += 2;
    }
    
    if (metadata.metrics.complexity_score > 30) {
        priority += 1;
    }
    
    std::string comp_lower = metadata.class_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    if (comp_lower.find("player") != std::string::npos || comp_lower.find("controller") != std::string::npos) {
        priority += 3;
    } else if (comp_lower.find("manager") != std::string::npos) {
        priority += 2;
    }
    
    return std::min(5, priority);
}

std::vector<DataFlowPath> DataFlowAnalyzer::traceDataPaths(const std::vector<DataFlowNode>& nodes) const {
    std::vector<DataFlowPath> paths;
    
    std::map<std::string, DataFlowNode> node_map;
    for (const auto& node : nodes) {
        node_map[node.component_name] = node;
    }
    
    for (const auto& start_node : nodes) {
        if (start_node.direction == DataFlowDirection::Input) {
            std::vector<std::string> visited;
            std::vector<std::string> current_path;
            
            std::function<void(const std::string&)> tracePath = [&](const std::string& comp_name) {
                if (std::find(visited.begin(), visited.end(), comp_name) != visited.end()) {
                    return;
                }
                
                visited.push_back(comp_name);
                current_path.push_back(comp_name);
                
                auto node_it = node_map.find(comp_name);
                if (node_it != node_map.end()) {
                    for (const auto& connected : node_it->second.connected_components) {
                        if (std::find(visited.begin(), visited.end(), connected) == visited.end()) {
                            tracePath(connected);
                        }
                    }
                }
                
                if (current_path.size() >= 2) {
                    DataFlowPath path;
                    path.path_components = current_path;
                    path.data_description = start_node.data_type + " flow";
                    path.flow_type = identifyFlowType(path);
                    
                    paths.push_back(path);
                }
                
                current_path.pop_back();
            };
            
            tracePath(start_node.component_name);
        }
    }
    
    std::sort(paths.begin(), paths.end(),
              [](const DataFlowPath& a, const DataFlowPath& b) {
                  return a.path_components.size() > b.path_components.size();
              });
    
    size_t max_paths = std::min(static_cast<size_t>(10), paths.size());
    paths.resize(max_paths);
    
    return paths;
}

std::string DataFlowAnalyzer::identifyFlowType(const DataFlowPath& path) const {
    if (path.path_components.size() < 2) {
        return "Simple";
    }
    
    bool has_input = false;
    bool has_processing = false;
    bool has_output = false;
    
    for (const auto& comp : path.path_components) {
        std::string comp_lower = comp;
        std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
        
        if (comp_lower.find("input") != std::string::npos || comp_lower.find("controller") != std::string::npos) {
            has_input = true;
        } else if (comp_lower.find("ui") != std::string::npos || comp_lower.find("display") != std::string::npos) {
            has_output = true;
        } else {
            has_processing = true;
        }
    }
    
    if (has_input && has_processing && has_output) {
        return "Input → Processing → Output";
    } else if (has_input && has_processing) {
        return "Input → Processing";
    } else if (has_processing && has_output) {
        return "Processing → Output";
    } else if (path.path_components.size() > 3) {
        return "Complex Processing";
    } else {
        return "Direct Communication";
    }
}

float DataFlowAnalyzer::calculateCriticality(const DataFlowPath& path,
                                            const Metadata::ProjectMetadata& project_metadata) const {
    float criticality = 0.5f;
    
    criticality += path.path_components.size() * 0.1f;
    
    for (const auto& comp : path.path_components) {
        auto comp_it = project_metadata.components.find(comp);
        if (comp_it != project_metadata.components.end()) {
            criticality += comp_it->second.dependents.size() * 0.05f;
            
            if (comp_it->second.metrics.complexity_score > 30) {
                criticality += 0.1f;
            }
        }
    }
    
    std::string first_comp_lower = path.path_components[0];
    std::transform(first_comp_lower.begin(), first_comp_lower.end(), first_comp_lower.begin(), ::tolower);
    
    if (first_comp_lower.find("player") != std::string::npos) {
        criticality += 0.3f;
    } else if (first_comp_lower.find("manager") != std::string::npos) {
        criticality += 0.2f;
    }
    
    return std::min(1.0f, criticality);
}

std::string DataFlowAnalyzer::determineDominantPattern(const std::vector<DataFlowPath>& flows) const {
    std::map<std::string, int> pattern_count;
    
    for (const auto& flow : flows) {
        pattern_count[flow.flow_type]++;
    }
    
    std::string dominant_pattern = "Unknown";
    int max_count = 0;
    
    for (const auto& pattern_pair : pattern_count) {
        if (pattern_pair.second > max_count) {
            max_count = pattern_pair.second;
            dominant_pattern = pattern_pair.first;
        }
    }
    
    return dominant_pattern;
}

std::vector<std::string> DataFlowAnalyzer::findDataHotspots(const std::vector<DataFlowNode>& nodes) const {
    std::map<std::string, int> component_frequency;
    
    for (const auto& node : nodes) {
        component_frequency[node.component_name] += node.connected_components.size();
        component_frequency[node.component_name] += node.priority_level;
    }
    
    std::vector<std::pair<std::string, int>> sorted_components(component_frequency.begin(), component_frequency.end());
    std::sort(sorted_components.begin(), sorted_components.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                  return a.second > b.second;
              });
    
    std::vector<std::string> hotspots;
    size_t max_hotspots = std::min(static_cast<size_t>(5), sorted_components.size());
    
    for (size_t i = 0; i < max_hotspots; ++i) {
        if (sorted_components[i].second > 3) {
            hotspots.push_back(sorted_components[i].first);
        }
    }
    
    return hotspots;
}

float DataFlowAnalyzer::calculateFlowEfficiency(const std::vector<DataFlowPath>& flows) const {
    if (flows.empty()) return 0.0f;
    
    float total_efficiency = 0.0f;
    
    for (const auto& flow : flows) {
        float flow_efficiency = 1.0f;
        
        if (flow.path_components.size() > 5) {
            flow_efficiency -= 0.2f;
        }
        
        if (!flow.bottlenecks.empty()) {
            flow_efficiency -= flow.bottlenecks.size() * 0.1f;
        }
        
        if (flow.flow_type == "Direct Communication") {
            flow_efficiency += 0.1f;
        } else if (flow.flow_type == "Complex Processing") {
            flow_efficiency -= 0.1f;
        }
        
        total_efficiency += std::max(0.0f, flow_efficiency);
    }
    
    return total_efficiency / flows.size();
}

bool DataFlowAnalyzer::isInputComponent(const std::string& component_name,
                                       const Metadata::ComponentMetadata& metadata) const {
    std::string comp_lower = component_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    bool has_input_keywords = (comp_lower.find("input") != std::string::npos ||
                              comp_lower.find("controller") != std::string::npos ||
                              comp_lower.find("player") != std::string::npos);
    
    bool has_update_method = false;
    for (const auto& method : metadata.lifecycle.methods) {
        if (method == "Update") {
            has_update_method = true;
            break;
        }
    }
    
    return has_input_keywords || (has_update_method && metadata.dependencies.empty());
}

bool DataFlowAnalyzer::isProcessingComponent(const std::string& component_name,
                                           const Metadata::ComponentMetadata& metadata) const {
    std::string comp_lower = component_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    bool has_processing_keywords = (comp_lower.find("logic") != std::string::npos ||
                                   comp_lower.find("behavior") != std::string::npos ||
                                   comp_lower.find("ai") != std::string::npos ||
                                   comp_lower.find("physics") != std::string::npos);
    
    bool has_dependencies_and_dependents = (!metadata.dependencies.empty() && !metadata.dependents.empty());
    
    return has_processing_keywords || has_dependencies_and_dependents;
}

bool DataFlowAnalyzer::isOutputComponent(const std::string& component_name,
                                        const Metadata::ComponentMetadata& metadata) const {
    std::string comp_lower = component_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    bool has_output_keywords = (comp_lower.find("ui") != std::string::npos ||
                               comp_lower.find("display") != std::string::npos ||
                               comp_lower.find("render") != std::string::npos ||
                               comp_lower.find("audio") != std::string::npos ||
                               comp_lower.find("visual") != std::string::npos);
    
    bool has_only_dependents = (!metadata.dependents.empty() && metadata.dependencies.empty());
    
    return has_output_keywords || has_only_dependents;
}

std::vector<std::string> DataFlowAnalyzer::identifyBottlenecks(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> bottlenecks;
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp = comp_pair.second;
        
        if (comp.dependents.size() > 5) {
            bottlenecks.push_back(comp.class_name + " (high fan-out: " + std::to_string(comp.dependents.size()) + " dependents)");
        }
        
        if (comp.metrics.complexity_score > 50) {
            bottlenecks.push_back(comp.class_name + " (high complexity: " + std::to_string(comp.metrics.complexity_score) + ")");
        }
        
        if (comp.dependencies.size() > 8) {
            bottlenecks.push_back(comp.class_name + " (high coupling: " + std::to_string(comp.dependencies.size()) + " dependencies)");
        }
    }
    
    return bottlenecks;
}

std::string DataFlowAnalyzer::generateDataFlowSummary(const Metadata::ProjectMetadata& project_metadata) const {
    std::stringstream summary;
    
    auto insights = analyzeDataFlow(project_metadata);
    
    summary << "Data Flow Analysis Summary\n";
    summary << "=========================\n\n";
    
    summary << "Dominant Flow Pattern: " << insights.dominant_flow_pattern << "\n";
    summary << "Flow Efficiency: " << static_cast<int>(insights.flow_efficiency_score * 100) << "%\n\n";
    
    if (!insights.primary_flows.empty()) {
        summary << "Primary Data Flows:\n";
        for (size_t i = 0; i < insights.primary_flows.size(); ++i) {
            const auto& flow = insights.primary_flows[i];
            summary << "  " << (i + 1) << ". " << flow.flow_type << ": ";
            for (size_t j = 0; j < flow.path_components.size(); ++j) {
                if (j > 0) summary << " → ";
                summary << flow.path_components[j];
            }
            summary << "\n";
        }
        summary << "\n";
    }
    
    if (!insights.data_hotspots.empty()) {
        summary << "Data Hotspots (High Activity Components):\n";
        for (const auto& hotspot : insights.data_hotspots) {
            summary << "  - " << hotspot << "\n";
        }
        summary << "\n";
    }
    
    if (!insights.isolated_components.empty()) {
        summary << "Isolated Components:\n";
        for (const auto& isolated : insights.isolated_components) {
            summary << "  - " << isolated << "\n";
        }
        summary << "\n";
    }
    
    auto bottlenecks = identifyBottlenecks(project_metadata);
    if (!bottlenecks.empty()) {
        summary << "Potential Bottlenecks:\n";
        for (const auto& bottleneck : bottlenecks) {
            summary << "  - " << bottleneck << "\n";
        }
    }
    
    return summary.str();
}

std::string DataFlowAnalyzer::generateFlowOptimizationReport(const DataFlowInsights& insights) const {
    std::stringstream report;
    
    report << "Data Flow Optimization Report\n";
    report << "============================\n\n";
    
    report << "Current Efficiency: " << static_cast<int>(insights.flow_efficiency_score * 100) << "%\n\n";
    
    if (insights.flow_efficiency_score < 0.7f) {
        report << "Optimization Recommendations:\n";
        
        if (!insights.data_hotspots.empty()) {
            report << "1. Reduce Load on Hotspots:\n";
            for (const auto& hotspot : insights.data_hotspots) {
                report << "   - Consider breaking down " << hotspot << " into smaller components\n";
                report << "   - Use caching to reduce repeated data access\n";
            }
            report << "\n";
        }
        
        if (!insights.isolated_components.empty()) {
            report << "2. Better Integration:\n";
            report << "   - Consider removing unused components: ";
            for (size_t i = 0; i < insights.isolated_components.size(); ++i) {
                if (i > 0) report << ", ";
                report << insights.isolated_components[i];
            }
            report << "\n\n";
        }
        
        bool has_complex_flows = false;
        for (const auto& flow : insights.primary_flows) {
            if (flow.path_components.size() > 4) {
                has_complex_flows = true;
                break;
            }
        }
        
        if (has_complex_flows) {
            report << "3. Simplify Complex Flows:\n";
            report << "   - Use event-driven communication to reduce direct dependencies\n";
            report << "   - Introduce mediator components for complex interactions\n";
            report << "   - Consider using Unity's messaging system\n\n";
        }
    } else {
        report << "Data flow is well-optimized. Consider these minor improvements:\n";
        report << "- Monitor hotspot components for performance\n";
        report << "- Maintain clean separation between input, processing, and output\n";
        report << "- Continue using established patterns for consistency\n";
    }
    
    return report.str();
}

} // namespace Context
} // namespace UnityContextGen