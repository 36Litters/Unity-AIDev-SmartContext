#pragma once

#include "ProjectMetadata.h"
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>

namespace UnityContextGen {
namespace Metadata {

class JSONExporter {
public:
    JSONExporter();
    ~JSONExporter() = default;

    bool exportProjectMetadata(const ProjectMetadata& metadata, const std::string& output_path) const;
    bool exportComponentMetadata(const ComponentMetadata& metadata, const std::string& output_path) const;
    bool exportSummary(const ProjectMetadata& metadata, const std::string& output_path) const;
    
    nlohmann::json createClaudeCodeOptimizedJSON(const ProjectMetadata& metadata) const;
    nlohmann::json createComponentAnalysisJSON(const std::vector<ComponentMetadata>& components) const;
    nlohmann::json createArchitectureOverviewJSON(const ProjectMetadata& metadata) const;
    
    std::string formatForLLMPrompt(const ProjectMetadata& metadata) const;
    std::string formatComponentForLLMPrompt(const ComponentMetadata& component) const;
    
    bool writeToFile(const nlohmann::json& json_data, const std::string& file_path) const;
    std::string prettifyJSON(const nlohmann::json& json_data) const;

private:
    nlohmann::json createComponentJSON(const ComponentMetadata& metadata) const;
    nlohmann::json createSystemGroupsJSON(const ProjectMetadata& metadata) const;
    nlohmann::json createDependencyGraphJSON(const ProjectMetadata& metadata) const;
    nlohmann::json createPatternAnalysisJSON(const ProjectMetadata& metadata) const;
    
    std::string escapeForPrompt(const std::string& text) const;
    std::string formatResponsibilityBlocks(const ComponentMetadata::ResponsibilityBlocks& blocks) const;
    std::string formatLifecycleInfo(const ComponentMetadata::LifecycleInfo& lifecycle) const;
};

} // namespace Metadata
} // namespace UnityContextGen