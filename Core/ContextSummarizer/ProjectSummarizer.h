#pragma once

#include "ArchitectureAnalyzer.h"
#include "DataFlowAnalyzer.h"
#include "../MetadataGenerator/ProjectMetadata.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace UnityContextGen {
namespace Context {

struct ProjectContext {
    struct OverallSummary {
        std::string project_type;
        std::string architecture_pattern;
        std::string complexity_level;
        std::string development_stage;
        std::vector<std::string> key_characteristics;
        float quality_score;
    } summary;
    
    struct DevelopmentGuidance {
        std::vector<std::string> coding_conventions;
        std::vector<std::string> architectural_principles;
        std::vector<std::string> performance_guidelines;
        std::vector<std::string> unity_best_practices;
        std::string recommended_approach;
    } guidance;
    
    struct LLMOptimization {
        std::string context_prompt;
        std::string component_template;
        std::vector<std::string> key_patterns;
        std::map<std::string, std::string> component_purposes;
        std::string development_style;
    } llm_context;
    
    ArchitectureInsight architecture;
    DataFlowInsights data_flow;
    std::vector<std::string> critical_components;
    std::vector<std::string> improvement_priorities;
};

class ProjectSummarizer {
public:
    ProjectSummarizer();
    ~ProjectSummarizer() = default;

    ProjectContext generateProjectContext(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::string generateLLMContextPrompt(const ProjectContext& context) const;
    std::string generateComponentGuidance(const std::string& component_name,
                                        const ProjectContext& context) const;
    std::string generateArchitectureOverview(const ProjectContext& context) const;
    
    nlohmann::json exportContextToJSON(const ProjectContext& context) const;
    nlohmann::json exportLLMOptimizedJSON(const ProjectContext& context) const;
    
    std::string generateDevelopmentGuidelines(const ProjectContext& context) const;
    std::string generateQuickReference(const ProjectContext& context) const;

private:
    ArchitectureAnalyzer m_architecture_analyzer;
    DataFlowAnalyzer m_data_flow_analyzer;
    
    std::string inferDevelopmentStage(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> extractKeyCharacteristics(const Metadata::ProjectMetadata& project_metadata) const;
    float calculateOverallQuality(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::vector<std::string> generateCodingConventions(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> generateArchitecturalPrinciples(const ArchitectureInsight& architecture) const;
    std::vector<std::string> generatePerformanceGuidelines(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> generateUnityBestPractices(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::string generateContextPrompt(const Metadata::ProjectMetadata& project_metadata,
                                     const ArchitectureInsight& architecture,
                                     const DataFlowInsights& data_flow) const;
    
    std::string generateComponentTemplate(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> extractKeyPatterns(const Metadata::ProjectMetadata& project_metadata) const;
    std::string inferDevelopmentStyle(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::vector<std::string> identifyCriticalComponents(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> prioritizeImprovements(const Metadata::ProjectMetadata& project_metadata,
                                                   const ArchitectureInsight& architecture) const;
    
    std::string formatComponentPurposes(const std::map<std::string, std::string>& purposes) const;
    std::string formatGuidanceList(const std::vector<std::string>& items, const std::string& prefix = "") const;
};

} // namespace Context
} // namespace UnityContextGen