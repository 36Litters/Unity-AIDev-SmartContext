#include "JSONExporter.h"
#include <iostream>
#include <iomanip>

namespace UnityContextGen {
namespace Metadata {

JSONExporter::JSONExporter() {
}

bool JSONExporter::exportProjectMetadata(const ProjectMetadata& metadata, const std::string& output_path) const {
    ProjectMetadataGenerator generator;
    nlohmann::json json_data = createClaudeCodeOptimizedJSON(metadata);
    return writeToFile(json_data, output_path);
}

bool JSONExporter::exportComponentMetadata(const ComponentMetadata& metadata, const std::string& output_path) const {
    nlohmann::json json_data = createComponentJSON(metadata);
    return writeToFile(json_data, output_path);
}

bool JSONExporter::exportSummary(const ProjectMetadata& metadata, const std::string& output_path) const {
    nlohmann::json summary;
    
    summary["project_info"] = {
        {"game_type", metadata.project_summary.game_type},
        {"architecture_pattern", metadata.project_summary.architecture_pattern},
        {"complexity_level", metadata.project_summary.complexity_level},
        {"total_components", metadata.project_summary.total_components}
    };
    
    summary["key_insights"] = {
        {"dominant_patterns", metadata.patterns.dominant_patterns},
        {"core_systems", metadata.systems.core_systems},
        {"quality_scores", {
            {"maintainability", metadata.quality.maintainability_score},
            {"testability", metadata.quality.testability_score},
            {"performance", metadata.quality.performance_score}
        }}
    };
    
    summary["recommendations"] = metadata.quality.improvement_suggestions;
    
    return writeToFile(summary, output_path);
}

nlohmann::json JSONExporter::createClaudeCodeOptimizedJSON(const ProjectMetadata& metadata) const {
    nlohmann::json optimized;
    
    optimized["project_context"] = {
        {"type", metadata.project_summary.game_type},
        {"architecture", metadata.project_summary.architecture_pattern},
        {"complexity", metadata.project_summary.complexity_level},
        {"key_systems", metadata.project_summary.key_systems}
    };
    
    optimized["component_overview"] = nlohmann::json::array();
    for (const auto& comp_pair : metadata.components) {
        const auto& comp = comp_pair.second;
        optimized["component_overview"].push_back({
            {"name", comp.class_name},
            {"purpose", comp.purpose},
            {"dependencies", comp.dependencies},
            {"responsibility_summary", comp.responsibility_blocks.block_order},
            {"complexity_score", comp.metrics.complexity_score}
        });
    }
    
    optimized["system_architecture"] = createSystemGroupsJSON(metadata);
    optimized["dependency_graph"] = createDependencyGraphJSON(metadata);
    optimized["design_patterns"] = createPatternAnalysisJSON(metadata);
    
    optimized["development_context"] = {
        {"code_style", "Unity C# MonoBehaviour patterns"},
        {"conventions", {
            "Use Unity lifecycle methods appropriately",
            "Follow component-based architecture",
            "Minimize dependencies between components",
            "Use SerializeField for inspector-visible fields"
        }},
        {"performance_considerations", {
            "Avoid expensive operations in Update methods",
            "Use object pooling for frequently created/destroyed objects",
            "Cache component references in Awake or Start"
        }}
    };
    
    optimized["llm_guidance"] = {
        {"when_adding_features", "Follow existing patterns and maintain component separation"},
        {"code_generation_style", "Match existing method naming and structure"},
        {"dependency_management", "Use GetComponent calls sparingly, prefer SerializeField references"},
        {"unity_best_practices", "Respect Unity execution order and lifecycle methods"}
    };
    
    return optimized;
}

nlohmann::json JSONExporter::createComponentAnalysisJSON(const std::vector<ComponentMetadata>& components) const {
    nlohmann::json analysis;
    
    analysis["components"] = nlohmann::json::array();
    
    for (const auto& comp : components) {
        analysis["components"].push_back(createComponentJSON(comp));
    }
    
    analysis["statistics"] = {
        {"total_components", components.size()},
        {"average_complexity", 0},
        {"high_complexity_components", nlohmann::json::array()}
    };
    
    float total_complexity = 0;
    for (const auto& comp : components) {
        total_complexity += comp.metrics.complexity_score;
        if (comp.metrics.complexity_score > 50) {
            analysis["statistics"]["high_complexity_components"].push_back(comp.class_name);
        }
    }
    
    if (!components.empty()) {
        analysis["statistics"]["average_complexity"] = total_complexity / components.size();
    }
    
    return analysis;
}

nlohmann::json JSONExporter::createArchitectureOverviewJSON(const ProjectMetadata& metadata) const {
    nlohmann::json overview;
    
    overview["architecture_summary"] = {
        {"pattern", metadata.project_summary.architecture_pattern},
        {"game_type", metadata.project_summary.game_type},
        {"complexity", metadata.project_summary.complexity_level}
    };
    
    overview["system_breakdown"] = createSystemGroupsJSON(metadata);
    overview["quality_assessment"] = {
        {"maintainability", metadata.quality.maintainability_score},
        {"testability", metadata.quality.testability_score},
        {"performance", metadata.quality.performance_score},
        {"architecture", metadata.quality.architecture_score}
    };
    
    overview["design_patterns"] = createPatternAnalysisJSON(metadata);
    overview["recommendations"] = metadata.quality.improvement_suggestions;
    
    return overview;
}

std::string JSONExporter::formatForLLMPrompt(const ProjectMetadata& metadata) const {
    std::stringstream prompt;
    
    prompt << "Project Context:\n";
    prompt << "================\n";
    prompt << "Type: " << metadata.project_summary.game_type << "\n";
    prompt << "Architecture: " << metadata.project_summary.architecture_pattern << "\n";
    prompt << "Complexity: " << metadata.project_summary.complexity_level << "\n\n";
    
    prompt << "Key Systems:\n";
    for (const auto& system : metadata.project_summary.key_systems) {
        prompt << "- " << system << "\n";
    }
    prompt << "\n";
    
    prompt << "Component Overview:\n";
    prompt << "==================\n";
    for (const auto& comp_pair : metadata.components) {
        const auto& comp = comp_pair.second;
        prompt << comp.class_name << ": " << comp.purpose << "\n";
        
        if (!comp.dependencies.empty()) {
            prompt << "  Dependencies: ";
            for (size_t i = 0; i < comp.dependencies.size(); ++i) {
                if (i > 0) prompt << ", ";
                prompt << comp.dependencies[i];
            }
            prompt << "\n";
        }
        
        prompt << "  Responsibilities: " << formatResponsibilityBlocks(comp.responsibility_blocks) << "\n";
        prompt << "\n";
    }
    
    if (!metadata.patterns.dominant_patterns.empty()) {
        prompt << "Dominant Design Patterns:\n";
        for (const auto& pattern : metadata.patterns.dominant_patterns) {
            prompt << "- " << pattern << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "Development Guidelines:\n";
    prompt << "======================\n";
    prompt << "- Follow existing component structure and naming conventions\n";
    prompt << "- Respect Unity lifecycle method execution order\n";
    prompt << "- Maintain low coupling between components\n";
    prompt << "- Use appropriate Unity patterns for the identified game type\n";
    
    return prompt.str();
}

std::string JSONExporter::formatComponentForLLMPrompt(const ComponentMetadata& component) const {
    std::stringstream prompt;
    
    prompt << "Component: " << component.class_name << "\n";
    prompt << "Purpose: " << component.purpose << "\n";
    prompt << "File: " << component.file_path << "\n";
    prompt << "Lines: " << component.start_line << "-" << component.end_line << "\n\n";
    
    if (!component.dependencies.empty()) {
        prompt << "Dependencies: ";
        for (size_t i = 0; i < component.dependencies.size(); ++i) {
            if (i > 0) prompt << ", ";
            prompt << component.dependencies[i];
        }
        prompt << "\n\n";
    }
    
    prompt << "Lifecycle Methods:\n";
    prompt << formatLifecycleInfo(component.lifecycle);
    prompt << "\n";
    
    prompt << "Responsibility Blocks:\n";
    prompt << formatResponsibilityBlocks(component.responsibility_blocks);
    prompt << "\n";
    
    if (!component.design_patterns.empty()) {
        prompt << "Design Patterns: ";
        for (size_t i = 0; i < component.design_patterns.size(); ++i) {
            if (i > 0) prompt << ", ";
            prompt << component.design_patterns[i];
        }
        prompt << "\n\n";
    }
    
    prompt << "Complexity Score: " << component.metrics.complexity_score << "\n";
    
    return prompt.str();
}

nlohmann::json JSONExporter::createComponentJSON(const ComponentMetadata& metadata) const {
    nlohmann::json component;
    
    component["class_name"] = metadata.class_name;
    component["file_path"] = metadata.file_path;
    component["purpose"] = metadata.purpose;
    component["dependencies"] = metadata.dependencies;
    component["dependents"] = metadata.dependents;
    
    component["lifecycle"] = {
        {"methods", metadata.lifecycle.methods},
        {"method_purposes", metadata.lifecycle.method_purposes},
        {"execution_order", metadata.lifecycle.execution_order},
        {"data_flow", metadata.lifecycle.data_flow}
    };
    
    component["responsibility_blocks"] = {
        {"blocks", metadata.responsibility_blocks.blocks},
        {"block_order", metadata.responsibility_blocks.block_order}
    };
    
    component["serialized_fields"] = metadata.serialized_fields;
    component["attributes"] = metadata.attributes;
    component["design_patterns"] = metadata.design_patterns;
    
    component["metrics"] = {
        {"complexity_score", metadata.metrics.complexity_score},
        {"dependency_count", metadata.metrics.dependency_count},
        {"method_count", metadata.metrics.method_count},
        {"unity_method_count", metadata.metrics.unity_method_count},
        {"pattern_confidence", metadata.metrics.pattern_confidence}
    };
    
    component["location"] = {
        {"start_line", metadata.start_line},
        {"end_line", metadata.end_line}
    };
    
    return component;
}

nlohmann::json JSONExporter::createSystemGroupsJSON(const ProjectMetadata& metadata) const {
    nlohmann::json systems;
    
    systems["groups"] = metadata.systems.system_groups;
    systems["core_systems"] = metadata.systems.core_systems;
    systems["support_systems"] = metadata.systems.support_systems;
    systems["complexity_scores"] = metadata.systems.system_complexity;
    
    return systems;
}

nlohmann::json JSONExporter::createDependencyGraphJSON(const ProjectMetadata& metadata) const {
    nlohmann::json dependencies;
    
    dependencies["clusters"] = metadata.dependencies.dependency_clusters;
    dependencies["high_coupling"] = metadata.dependencies.high_coupling_components;
    dependencies["isolated"] = metadata.dependencies.isolated_components;
    dependencies["has_cycles"] = metadata.dependencies.has_circular_dependencies;
    dependencies["topological_order"] = metadata.dependencies.topological_order;
    
    return dependencies;
}

nlohmann::json JSONExporter::createPatternAnalysisJSON(const ProjectMetadata& metadata) const {
    nlohmann::json patterns;
    
    patterns["frequency"] = metadata.patterns.pattern_frequency;
    patterns["dominant"] = metadata.patterns.dominant_patterns;
    patterns["consistency_score"] = metadata.patterns.pattern_consistency_score;
    patterns["recommended"] = metadata.patterns.recommended_patterns;
    
    return patterns;
}

bool JSONExporter::writeToFile(const nlohmann::json& json_data, const std::string& file_path) const {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << file_path << std::endl;
            return false;
        }
        
        file << std::setw(2) << json_data << std::endl;
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing JSON to file: " << e.what() << std::endl;
        return false;
    }
}

std::string JSONExporter::prettifyJSON(const nlohmann::json& json_data) const {
    return json_data.dump(2);
}

std::string JSONExporter::escapeForPrompt(const std::string& text) const {
    std::string escaped = text;
    
    size_t pos = 0;
    while ((pos = escaped.find('\n', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\n");
        pos += 2;
    }
    
    pos = 0;
    while ((pos = escaped.find('\t', pos)) != std::string::npos) {
        escaped.replace(pos, 1, "\\t");
        pos += 2;
    }
    
    return escaped;
}

std::string JSONExporter::formatResponsibilityBlocks(const ComponentMetadata::ResponsibilityBlocks& blocks) const {
    std::stringstream formatted;
    
    for (size_t i = 0; i < blocks.block_order.size(); ++i) {
        if (i > 0) formatted << "; ";
        
        const std::string& block_name = blocks.block_order[i];
        auto it = blocks.blocks.find(block_name);
        if (it != blocks.blocks.end()) {
            formatted << block_name << " (" << it->second << ")";
        }
    }
    
    return formatted.str();
}

std::string JSONExporter::formatLifecycleInfo(const ComponentMetadata::LifecycleInfo& lifecycle) const {
    std::stringstream formatted;
    
    for (size_t i = 0; i < lifecycle.execution_order.size(); ++i) {
        if (i > 0) formatted << " → ";
        
        const std::string& method = lifecycle.execution_order[i];
        formatted << method;
        
        auto purpose_it = lifecycle.method_purposes.find(method);
        if (purpose_it != lifecycle.method_purposes.end()) {
            formatted << " (" << purpose_it->second << ")";
        }
    }
    
    return formatted.str();
}

} // namespace Metadata
} // namespace UnityContextGen