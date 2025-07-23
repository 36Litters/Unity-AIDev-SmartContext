#include "ProjectSummarizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace UnityContextGen {
namespace Context {

ProjectSummarizer::ProjectSummarizer() {
}

ProjectContext ProjectSummarizer::generateProjectContext(const Metadata::ProjectMetadata& project_metadata) const {
    ProjectContext context;
    
    context.summary.project_type = project_metadata.project_summary.game_type;
    context.summary.architecture_pattern = project_metadata.project_summary.architecture_pattern;
    context.summary.complexity_level = project_metadata.project_summary.complexity_level;
    context.summary.development_stage = inferDevelopmentStage(project_metadata);
    context.summary.key_characteristics = extractKeyCharacteristics(project_metadata);
    context.summary.quality_score = calculateOverallQuality(project_metadata);
    
    context.architecture = m_architecture_analyzer.analyzeArchitecture(project_metadata);
    context.data_flow = m_data_flow_analyzer.analyzeDataFlow(project_metadata);
    
    context.guidance.coding_conventions = generateCodingConventions(project_metadata);
    context.guidance.architectural_principles = generateArchitecturalPrinciples(context.architecture);
    context.guidance.performance_guidelines = generatePerformanceGuidelines(project_metadata);
    context.guidance.unity_best_practices = generateUnityBestPractices(project_metadata);
    context.guidance.recommended_approach = "Follow established patterns and maintain component separation";
    
    context.llm_context.context_prompt = generateContextPrompt(project_metadata, context.architecture, context.data_flow);
    context.llm_context.component_template = generateComponentTemplate(project_metadata);
    context.llm_context.key_patterns = extractKeyPatterns(project_metadata);
    
    for (const auto& comp_pair : project_metadata.components) {
        context.llm_context.component_purposes[comp_pair.first] = comp_pair.second.purpose;
    }
    
    context.llm_context.development_style = inferDevelopmentStyle(project_metadata);
    
    context.critical_components = identifyCriticalComponents(project_metadata);
    context.improvement_priorities = prioritizeImprovements(project_metadata, context.architecture);
    
    return context;
}

std::string ProjectSummarizer::generateLLMContextPrompt(const ProjectContext& context) const {
    std::stringstream prompt;
    
    prompt << "# Unity Project Context\n\n";
    
    prompt << "## Project Overview\n";
    prompt << "**Type:** " << context.summary.project_type << "\n";
    prompt << "**Architecture:** " << context.summary.architecture_pattern << "\n";
    prompt << "**Complexity:** " << context.summary.complexity_level << "\n";
    prompt << "**Stage:** " << context.summary.development_stage << "\n";
    prompt << "**Quality Score:** " << static_cast<int>(context.summary.quality_score) << "%\n\n";
    
    if (!context.summary.key_characteristics.empty()) {
        prompt << "**Key Characteristics:**\n";
        for (const auto& characteristic : context.summary.key_characteristics) {
            prompt << "- " << characteristic << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "## Architecture Insights\n";
    prompt << "**Primary Pattern:** " << context.architecture.description << "\n";
    prompt << "**Confidence:** " << static_cast<int>(context.architecture.confidence_score * 100) << "%\n\n";
    
    if (!context.architecture.evidence.empty()) {
        prompt << "**Evidence:**\n";
        for (const auto& evidence : context.architecture.evidence) {
            prompt << "- " << evidence << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "## Data Flow\n";
    prompt << "**Dominant Pattern:** " << context.data_flow.dominant_flow_pattern << "\n";
    prompt << "**Efficiency:** " << static_cast<int>(context.data_flow.flow_efficiency_score * 100) << "%\n\n";
    
    if (!context.data_flow.primary_flows.empty()) {
        prompt << "**Primary Data Flows:**\n";
        for (size_t i = 0; i < context.data_flow.primary_flows.size(); ++i) {
            const auto& flow = context.data_flow.primary_flows[i];
            prompt << (i + 1) << ". " << flow.flow_type << ": ";
            for (size_t j = 0; j < flow.path_components.size(); ++j) {
                if (j > 0) prompt << " → ";
                prompt << flow.path_components[j];
            }
            prompt << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "## Component Overview\n";
    for (const auto& purpose_pair : context.llm_context.component_purposes) {
        prompt << "**" << purpose_pair.first << ":** " << purpose_pair.second << "\n";
    }
    prompt << "\n";
    
    if (!context.critical_components.empty()) {
        prompt << "## Critical Components\n";
        for (const auto& component : context.critical_components) {
            prompt << "- " << component << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "## Development Guidelines\n";
    prompt << "**Style:** " << context.llm_context.development_style << "\n\n";
    
    if (!context.llm_context.key_patterns.empty()) {
        prompt << "**Key Patterns to Follow:**\n";
        for (const auto& pattern : context.llm_context.key_patterns) {
            prompt << "- " << pattern << "\n";
        }
        prompt << "\n";
    }
    
    if (!context.guidance.coding_conventions.empty()) {
        prompt << "**Coding Conventions:**\n";
        for (const auto& convention : context.guidance.coding_conventions) {
            prompt << "- " << convention << "\n";
        }
        prompt << "\n";
    }
    
    if (!context.improvement_priorities.empty()) {
        prompt << "## Improvement Priorities\n";
        for (size_t i = 0; i < context.improvement_priorities.size(); ++i) {
            prompt << (i + 1) << ". " << context.improvement_priorities[i] << "\n";
        }
        prompt << "\n";
    }
    
    prompt << "## Recommended Approach\n";
    prompt << context.guidance.recommended_approach << "\n";
    
    return prompt.str();
}

std::string ProjectSummarizer::generateComponentGuidance(const std::string& component_name,
                                                        const ProjectContext& context) const {
    std::stringstream guidance;
    
    guidance << "# Component Guidance: " << component_name << "\n\n";
    
    auto purpose_it = context.llm_context.component_purposes.find(component_name);
    if (purpose_it != context.llm_context.component_purposes.end()) {
        guidance << "**Purpose:** " << purpose_it->second << "\n\n";
    }
    
    guidance << "## Development Context\n";
    guidance << "**Project Type:** " << context.summary.project_type << "\n";
    guidance << "**Architecture:** " << context.summary.architecture_pattern << "\n";
    guidance << "**Development Style:** " << context.llm_context.development_style << "\n\n";
    
    bool is_critical = std::find(context.critical_components.begin(), 
                                context.critical_components.end(), 
                                component_name) != context.critical_components.end();
    
    if (is_critical) {
        guidance << "⚠️ **Critical Component** - Extra care needed when modifying\n\n";
    }
    
    guidance << "## Guidelines for This Component\n";
    
    std::string comp_lower = component_name;
    std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
    
    if (comp_lower.find("player") != std::string::npos) {
        guidance << "- Handle input processing in Update method\n";
        guidance << "- Use FixedUpdate for physics-based movement\n";
        guidance << "- Cache component references in Awake/Start\n";
        guidance << "- Separate input handling from movement logic\n";
    } else if (comp_lower.find("enemy") != std::string::npos || comp_lower.find("ai") != std::string::npos) {
        guidance << "- Implement state-based AI behavior\n";
        guidance << "- Use coroutines for time-based actions\n";
        guidance << "- Consider using Unity's NavMesh for pathfinding\n";
        guidance << "- Optimize AI calculations for performance\n";
    } else if (comp_lower.find("ui") != std::string::npos) {
        guidance << "- Follow Unity UI best practices\n";
        guidance << "- Use events for UI interactions\n";
        guidance << "- Separate UI logic from game logic\n";
        guidance << "- Consider using Unity's new UI system\n";
    } else if (comp_lower.find("manager") != std::string::npos) {
        guidance << "- Implement singleton pattern if global access needed\n";
        guidance << "- Use events for loose coupling\n";
        guidance << "- Handle initialization order carefully\n";
        guidance << "- Consider using ScriptableObjects for configuration\n";
    } else {
        guidance << "- Follow established project patterns\n";
        guidance << "- Maintain consistent naming conventions\n";
        guidance << "- Use appropriate Unity lifecycle methods\n";
        guidance << "- Keep responsibilities focused and clear\n";
    }
    
    guidance << "\n## Project-Specific Patterns\n";
    for (const auto& pattern : context.llm_context.key_patterns) {
        guidance << "- " << pattern << "\n";
    }
    
    return guidance.str();
}

std::string ProjectSummarizer::generateArchitectureOverview(const ProjectContext& context) const {
    std::stringstream overview;
    
    overview << "# Architecture Overview\n\n";
    
    overview << "## System Architecture\n";
    overview << "**Pattern:** " << context.architecture.description << "\n";
    overview << "**Confidence:** " << static_cast<int>(context.architecture.confidence_score * 100) << "%\n\n";
    
    if (!context.architecture.characteristics.empty()) {
        overview << "**Characteristics:**\n";
        for (const auto& characteristic : context.architecture.characteristics) {
            overview << "- " << characteristic << "\n";
        }
        overview << "\n";
    }
    
    overview << "## Data Flow Architecture\n";
    overview << "**Pattern:** " << context.data_flow.dominant_flow_pattern << "\n";
    overview << "**Efficiency:** " << static_cast<int>(context.data_flow.flow_efficiency_score * 100) << "%\n\n";
    
    if (!context.data_flow.data_hotspots.empty()) {
        overview << "**Data Hotspots:**\n";
        for (const auto& hotspot : context.data_flow.data_hotspots) {
            overview << "- " << hotspot << "\n";
        }
        overview << "\n";
    }
    
    overview << "## Architectural Principles\n";
    for (const auto& principle : context.guidance.architectural_principles) {
        overview << "- " << principle << "\n";
    }
    overview << "\n";
    
    overview << "## Quality Assessment\n";
    overview << "**Overall Quality:** " << static_cast<int>(context.summary.quality_score) << "%\n\n";
    
    if (!context.improvement_priorities.empty()) {
        overview << "**Improvement Priorities:**\n";
        for (size_t i = 0; i < std::min(static_cast<size_t>(3), context.improvement_priorities.size()); ++i) {
            overview << (i + 1) << ". " << context.improvement_priorities[i] << "\n";
        }
    }
    
    return overview.str();
}

std::string ProjectSummarizer::inferDevelopmentStage(const Metadata::ProjectMetadata& project_metadata) const {
    int component_count = project_metadata.project_summary.total_components;
    int system_count = project_metadata.systems.core_systems.size();
    float quality_score = calculateOverallQuality(project_metadata);
    
    if (component_count < 5) {
        return "Early Development";
    } else if (component_count < 15 && system_count < 3) {
        return "Prototype";
    } else if (quality_score > 80 && system_count >= 3) {
        return "Production Ready";
    } else if (quality_score > 60) {
        return "Active Development";
    } else {
        return "Needs Refactoring";
    }
}

std::vector<std::string> ProjectSummarizer::extractKeyCharacteristics(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> characteristics;
    
    if (project_metadata.project_summary.total_components > 20) {
        characteristics.push_back("Large-scale project with " + std::to_string(project_metadata.project_summary.total_components) + " components");
    }
    
    if (project_metadata.systems.core_systems.size() >= 4) {
        characteristics.push_back("Multi-system architecture");
    }
    
    if (!project_metadata.patterns.dominant_patterns.empty()) {
        characteristics.push_back("Uses design patterns: " + project_metadata.patterns.dominant_patterns[0]);
    }
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        characteristics.push_back("Contains circular dependencies");
    } else {
        characteristics.push_back("Clean dependency structure");
    }
    
    if (project_metadata.quality.performance_score > 85) {
        characteristics.push_back("Performance-optimized");
    } else if (project_metadata.quality.performance_score < 60) {
        characteristics.push_back("Performance needs attention");
    }
    
    return characteristics;
}

float ProjectSummarizer::calculateOverallQuality(const Metadata::ProjectMetadata& project_metadata) const {
    return (project_metadata.quality.maintainability_score +
            project_metadata.quality.testability_score +
            project_metadata.quality.performance_score +
            project_metadata.quality.architecture_score) / 4.0f;
}

std::vector<std::string> ProjectSummarizer::generateCodingConventions(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> conventions;
    
    conventions.push_back("Use PascalCase for class names and public methods");
    conventions.push_back("Use camelCase for private fields and local variables");
    conventions.push_back("Prefix private fields with 'm_' or underscore '_'");
    conventions.push_back("Use [SerializeField] for private fields exposed in Inspector");
    conventions.push_back("Cache component references in Awake() or Start()");
    conventions.push_back("Use null checks before component operations");
    
    if (project_metadata.project_summary.game_type.find("3D") != std::string::npos ||
        project_metadata.project_summary.game_type.find("FPS") != std::string::npos) {
        conventions.push_back("Use Vector3 for 3D positions and Quaternion for rotations");
    }
    
    if (project_metadata.quality.performance_score < 70) {
        conventions.push_back("Avoid expensive operations in Update() methods");
        conventions.push_back("Use object pooling for frequently created/destroyed objects");
    }
    
    return conventions;
}

std::vector<std::string> ProjectSummarizer::generateArchitecturalPrinciples(const ArchitectureInsight& architecture) const {
    std::vector<std::string> principles;
    
    principles.push_back("Single Responsibility: Each component should have one clear purpose");
    principles.push_back("Dependency Inversion: Depend on interfaces, not concrete implementations");
    principles.push_back("Open/Closed: Components should be open for extension, closed for modification");
    
    if (architecture.primary_type == ArchitectureType::ComponentBasedEntity) {
        principles.push_back("Component Composition: Favor composition over inheritance");
        principles.push_back("GameObject Organization: Group related components on same GameObject");
    }
    
    if (architecture.primary_type == ArchitectureType::EventDrivenArchitecture) {
        principles.push_back("Event-Driven Communication: Use events to decouple components");
        principles.push_back("Publisher-Subscriber: Implement loose coupling through events");
    }
    
    principles.push_back("Unity Lifecycle: Respect Unity method execution order");
    principles.push_back("Performance First: Consider performance implications of architectural decisions");
    
    return principles;
}

std::vector<std::string> ProjectSummarizer::generatePerformanceGuidelines(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> guidelines;
    
    int update_method_count = 0;
    for (const auto& comp_pair : project_metadata.components) {
        for (const auto& method : comp_pair.second.lifecycle.methods) {
            if (method == "Update") {
                update_method_count++;
            }
        }
    }
    
    if (update_method_count > 10) {
        guidelines.push_back("Minimize Update() method usage - consider event-driven alternatives");
        guidelines.push_back("Use coroutines for time-based operations instead of Update() checks");
    }
    
    guidelines.push_back("Cache component references instead of repeated GetComponent() calls");
    guidelines.push_back("Use object pooling for frequently instantiated objects");
    guidelines.push_back("Prefer FixedUpdate() for physics calculations");
    guidelines.push_back("Use Unity Profiler to identify performance bottlenecks");
    
    if (project_metadata.project_summary.game_type.find("Mobile") != std::string::npos) {
        guidelines.push_back("Optimize for mobile: reduce draw calls and texture memory");
        guidelines.push_back("Use LOD (Level of Detail) for complex models");
    }
    
    return guidelines;
}

std::vector<std::string> ProjectSummarizer::generateUnityBestPractices(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> practices;
    
    practices.push_back("Initialize in Awake(), start behaviors in Start()");
    practices.push_back("Use RequireComponent attribute for dependencies");
    practices.push_back("Implement proper null checking before component access");
    practices.push_back("Use Unity Events for inspector-configurable callbacks");
    practices.push_back("Organize scenes with empty GameObjects as organizational containers");
    
    if (project_metadata.systems.system_groups.size() > 3) {
        practices.push_back("Use managers for cross-system communication");
        practices.push_back("Implement proper initialization order for managers");
    }
    
    if (!project_metadata.patterns.dominant_patterns.empty()) {
        for (const auto& pattern : project_metadata.patterns.dominant_patterns) {
            if (pattern.find("Singleton") != std::string::npos) {
                practices.push_back("Use DontDestroyOnLoad for persistent singletons");
            }
        }
    }
    
    practices.push_back("Use ScriptableObjects for game data and configuration");
    practices.push_back("Leverage Unity's built-in components when possible");
    
    return practices;
}

std::string ProjectSummarizer::generateContextPrompt(const Metadata::ProjectMetadata& project_metadata,
                                                    const ArchitectureInsight& architecture,
                                                    const DataFlowInsights& data_flow) const {
    std::stringstream prompt;
    
    prompt << "Unity " << project_metadata.project_summary.game_type << " project using ";
    prompt << architecture.description << ". ";
    
    prompt << "Data flows primarily through " << data_flow.dominant_flow_pattern << " pattern. ";
    
    prompt << "Key systems: ";
    for (size_t i = 0; i < project_metadata.project_summary.key_systems.size(); ++i) {
        if (i > 0) prompt << ", ";
        prompt << project_metadata.project_summary.key_systems[i];
    }
    prompt << ". ";
    
    if (!project_metadata.patterns.dominant_patterns.empty()) {
        prompt << "Uses " << project_metadata.patterns.dominant_patterns[0] << " design pattern. ";
    }
    
    prompt << "Maintain existing patterns and component separation.";
    
    return prompt.str();
}

std::string ProjectSummarizer::generateComponentTemplate(const Metadata::ProjectMetadata& project_metadata) const {
    std::stringstream template_str;
    
    template_str << "Standard Unity MonoBehaviour component following project conventions:\n";
    template_str << "- Use [SerializeField] for inspector fields\n";
    template_str << "- Cache references in Awake()\n";
    template_str << "- Initialize behavior in Start()\n";
    
    if (project_metadata.project_summary.architecture_pattern.find("Component") != std::string::npos) {
        template_str << "- Implement single responsibility principle\n";
        template_str << "- Use composition over inheritance\n";
    }
    
    template_str << "- Follow Unity lifecycle method order\n";
    template_str << "- Include null checks for component references";
    
    return template_str.str();
}

std::vector<std::string> ProjectSummarizer::extractKeyPatterns(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> patterns;
    
    for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
        if (pattern_pair.second > 0) {
            patterns.push_back(pattern_pair.first);
        }
    }
    
    if (patterns.empty()) {
        patterns.push_back("Standard Unity MonoBehaviour pattern");
        patterns.push_back("Component-based architecture");
    }
    
    patterns.push_back("Unity lifecycle method usage");
    patterns.push_back("SerializeField for inspector exposure");
    
    return patterns;
}

std::string ProjectSummarizer::inferDevelopmentStyle(const Metadata::ProjectMetadata& project_metadata) const {
    if (project_metadata.patterns.pattern_consistency_score > 0.8f) {
        return "Consistent, pattern-driven development";
    } else if (project_metadata.quality.architecture_score > 80) {
        return "Architecture-focused development";
    } else if (project_metadata.quality.performance_score > 80) {
        return "Performance-optimized development";
    } else if (project_metadata.project_summary.complexity_level == "Simple") {
        return "Pragmatic, straightforward development";
    } else {
        return "Iterative development with room for improvement";
    }
}

std::vector<std::string> ProjectSummarizer::identifyCriticalComponents(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> critical;
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp = comp_pair.second;
        
        if (comp.dependents.size() > 5 || comp.metrics.complexity_score > 50) {
            critical.push_back(comp.class_name);
        }
        
        std::string comp_lower = comp.class_name;
        std::transform(comp_lower.begin(), comp_lower.end(), comp_lower.begin(), ::tolower);
        
        if (comp_lower.find("manager") != std::string::npos || 
            comp_lower.find("controller") != std::string::npos) {
            critical.push_back(comp.class_name);
        }
    }
    
    std::sort(critical.begin(), critical.end());
    critical.erase(std::unique(critical.begin(), critical.end()), critical.end());
    
    return critical;
}

std::vector<std::string> ProjectSummarizer::prioritizeImprovements(const Metadata::ProjectMetadata& project_metadata,
                                                                  const ArchitectureInsight& architecture) const {
    std::vector<std::string> priorities;
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        priorities.push_back("Resolve circular dependencies");
    }
    
    if (project_metadata.quality.performance_score < 60) {
        priorities.push_back("Optimize performance bottlenecks");
    }
    
    if (project_metadata.quality.maintainability_score < 60) {
        priorities.push_back("Improve code maintainability");
    }
    
    if (architecture.confidence_score < 0.6f) {
        priorities.push_back("Clarify architectural patterns");
    }
    
    if (!project_metadata.dependencies.high_coupling_components.empty()) {
        priorities.push_back("Reduce component coupling");
    }
    
    if (priorities.empty()) {
        priorities.push_back("Continue following established patterns");
        priorities.push_back("Monitor performance and maintainability");
    }
    
    return priorities;
}

nlohmann::json ProjectSummarizer::exportContextToJSON(const ProjectContext& context) const {
    nlohmann::json json_context;
    
    json_context["summary"] = {
        {"project_type", context.summary.project_type},
        {"architecture_pattern", context.summary.architecture_pattern},
        {"complexity_level", context.summary.complexity_level},
        {"development_stage", context.summary.development_stage},
        {"key_characteristics", context.summary.key_characteristics},
        {"quality_score", context.summary.quality_score}
    };
    
    json_context["architecture"] = {
        {"description", context.architecture.description},
        {"confidence_score", context.architecture.confidence_score},
        {"evidence", context.architecture.evidence},
        {"characteristics", context.architecture.characteristics}
    };
    
    json_context["data_flow"] = {
        {"dominant_pattern", context.data_flow.dominant_flow_pattern},
        {"efficiency_score", context.data_flow.flow_efficiency_score},
        {"data_hotspots", context.data_flow.data_hotspots},
        {"isolated_components", context.data_flow.isolated_components}
    };
    
    json_context["guidance"] = {
        {"coding_conventions", context.guidance.coding_conventions},
        {"architectural_principles", context.guidance.architectural_principles},
        {"performance_guidelines", context.guidance.performance_guidelines},
        {"unity_best_practices", context.guidance.unity_best_practices},
        {"recommended_approach", context.guidance.recommended_approach}
    };
    
    json_context["llm_context"] = {
        {"context_prompt", context.llm_context.context_prompt},
        {"component_template", context.llm_context.component_template},
        {"key_patterns", context.llm_context.key_patterns},
        {"component_purposes", context.llm_context.component_purposes},
        {"development_style", context.llm_context.development_style}
    };
    
    json_context["critical_components"] = context.critical_components;
    json_context["improvement_priorities"] = context.improvement_priorities;
    
    return json_context;
}

nlohmann::json ProjectSummarizer::exportLLMOptimizedJSON(const ProjectContext& context) const {
    nlohmann::json optimized;
    
    optimized["project_context"] = context.llm_context.context_prompt;
    optimized["development_style"] = context.llm_context.development_style;
    optimized["key_patterns"] = context.llm_context.key_patterns;
    
    optimized["component_guidance"] = nlohmann::json::object();
    for (const auto& purpose_pair : context.llm_context.component_purposes) {
        optimized["component_guidance"][purpose_pair.first] = {
            {"purpose", purpose_pair.second},
            {"is_critical", std::find(context.critical_components.begin(), 
                                    context.critical_components.end(), 
                                    purpose_pair.first) != context.critical_components.end()}
        };
    }
    
    optimized["coding_guidelines"] = {
        {"conventions", context.guidance.coding_conventions},
        {"unity_practices", context.guidance.unity_best_practices},
        {"performance", context.guidance.performance_guidelines}
    };
    
    optimized["architecture_info"] = {
        {"pattern", context.summary.architecture_pattern},
        {"confidence", context.architecture.confidence_score},
        {"data_flow", context.data_flow.dominant_flow_pattern}
    };
    
    optimized["improvement_focus"] = context.improvement_priorities;
    
    return optimized;
}

std::string ProjectSummarizer::generateDevelopmentGuidelines(const ProjectContext& context) const {
    std::stringstream guidelines;
    
    guidelines << "# Development Guidelines\n\n";
    
    guidelines << "## Project Context\n";
    guidelines << "- **Type:** " << context.summary.project_type << "\n";
    guidelines << "- **Architecture:** " << context.summary.architecture_pattern << "\n";
    guidelines << "- **Stage:** " << context.summary.development_stage << "\n";
    guidelines << "- **Style:** " << context.llm_context.development_style << "\n\n";
    
    guidelines << "## Coding Conventions\n";
    guidelines << formatGuidanceList(context.guidance.coding_conventions);
    
    guidelines << "## Architectural Principles\n";
    guidelines << formatGuidanceList(context.guidance.architectural_principles);
    
    guidelines << "## Performance Guidelines\n";
    guidelines << formatGuidanceList(context.guidance.performance_guidelines);
    
    guidelines << "## Unity Best Practices\n";
    guidelines << formatGuidanceList(context.guidance.unity_best_practices);
    
    return guidelines.str();
}

std::string ProjectSummarizer::generateQuickReference(const ProjectContext& context) const {
    std::stringstream reference;
    
    reference << "# Quick Reference\n\n";
    
    reference << "**Project:** " << context.summary.project_type << " | ";
    reference << "**Architecture:** " << context.summary.architecture_pattern << " | ";
    reference << "**Quality:** " << static_cast<int>(context.summary.quality_score) << "%\n\n";
    
    reference << "## Key Components\n";
    for (const auto& purpose_pair : context.llm_context.component_purposes) {
        reference << "- **" << purpose_pair.first << "**: " << purpose_pair.second << "\n";
    }
    reference << "\n";
    
    if (!context.critical_components.empty()) {
        reference << "## Critical Components ⚠️\n";
        for (const auto& component : context.critical_components) {
            reference << "- " << component << "\n";
        }
        reference << "\n";
    }
    
    reference << "## Key Patterns\n";
    for (const auto& pattern : context.llm_context.key_patterns) {
        reference << "- " << pattern << "\n";
    }
    reference << "\n";
    
    reference << "## Current Priorities\n";
    for (size_t i = 0; i < std::min(static_cast<size_t>(3), context.improvement_priorities.size()); ++i) {
        reference << (i + 1) << ". " << context.improvement_priorities[i] << "\n";
    }
    
    return reference.str();
}

std::string ProjectSummarizer::formatGuidanceList(const std::vector<std::string>& items, const std::string& prefix) const {
    std::stringstream formatted;
    for (const auto& item : items) {
        formatted << prefix << "- " << item << "\n";
    }
    formatted << "\n";
    return formatted.str();
}

} // namespace Context
} // namespace UnityContextGen