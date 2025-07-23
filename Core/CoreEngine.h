#pragma once

#include "TreeSitterEngine/CSharpParser.h"
#include "UnityAnalyzer/MonoBehaviourAnalyzer.h"
#include "UnityAnalyzer/ComponentDependencyAnalyzer.h"
#include "UnityAnalyzer/LifecycleAnalyzer.h"
#include "UnityAnalyzer/UnityPatternDetector.h"
#include "UnityAnalyzer/ScriptableObjectAnalyzer.h"
#include "MetadataGenerator/ProjectMetadata.h"
#include "ContextSummarizer/ProjectSummarizer.h"
#include "AI/ClaudeAnalyzer.h"
#include "AI/UnityAPIDetector.h"
#include "Configuration/AnalysisConfig.h"
#include <string>
#include <vector>
#include <memory>

namespace UnityContextGen {

struct AnalysisOptions {
    bool analyze_dependencies = true;
    bool analyze_lifecycle = true;
    bool detect_patterns = true;
    bool generate_metadata = true;
    bool generate_context = true;
    bool export_json = true;
    
    // Advanced analysis options
    bool analyze_scriptable_objects = true;
    bool analyze_asset_references = true;
    bool analyze_addressables = false;
    
    std::string output_directory = "./output";
    bool verbose_output = false;
};

struct AnalysisResult {
    bool success;
    std::string error_message;
    
    std::vector<Unity::MonoBehaviourInfo> monobehaviours;
    Unity::ComponentGraph dependency_graph;
    std::vector<Unity::LifecycleFlow> lifecycle_flows;
    std::vector<Unity::PatternInstance> patterns;
    
    // ScriptableObject analysis results
    std::vector<::Unity::ScriptableObjectInfo> scriptable_objects;
    std::vector<::Unity::AssetDependency> asset_dependencies;
    
    // AI analysis results
    std::vector<AI::UnityAPIUsage> api_usage;
    std::vector<AI::AIAnalysisResult> ai_analysis_results;
    
    Metadata::ProjectMetadata project_metadata;
    Context::ProjectContext project_context;
    
    std::vector<std::string> output_files;
    double analysis_duration_ms;
};

class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine() = default;

    AnalysisResult analyzeProject(const std::vector<std::string>& source_files, 
                                 const AnalysisOptions& options = AnalysisOptions()) const;
    
    AnalysisResult analyzeDirectory(const std::string& directory_path,
                                   const AnalysisOptions& options = AnalysisOptions()) const;
    
    AnalysisResult analyzeSingleFile(const std::string& file_path,
                                    const AnalysisOptions& options = AnalysisOptions()) const;
    
    bool exportResults(const AnalysisResult& result, 
                      const AnalysisOptions& options) const;
    
    std::string generateLLMPrompt(const AnalysisResult& result) const;
    std::string generateQuickSummary(const AnalysisResult& result) const;
    std::string generateDetailedReport(const AnalysisResult& result) const;
    
    void setVerbose(bool verbose) { m_verbose = verbose; }
    bool getVerbose() const { return m_verbose; }

private:
    bool m_verbose;
    
    mutable TreeSitter::CSharpParser m_parser;
    mutable Unity::MonoBehaviourAnalyzer m_mb_analyzer;
    mutable Unity::ComponentDependencyAnalyzer m_dep_analyzer;
    mutable Unity::LifecycleAnalyzer m_lifecycle_analyzer;
    mutable Unity::UnityPatternDetector m_pattern_detector;
    mutable ::Unity::ScriptableObjectAnalyzer m_scriptable_object_analyzer;
    mutable Metadata::ProjectMetadataGenerator m_metadata_generator;
    mutable Context::ProjectSummarizer m_context_summarizer;
    mutable AI::UnityAPIDetector m_api_detector;
    mutable std::unique_ptr<AI::ClaudeAnalyzer> m_claude_analyzer;
    
    std::vector<std::string> findCSharpFiles(const std::string& directory_path) const;
    bool isValidCSharpFile(const std::string& file_path) const;
    
    void logVerbose(const std::string& message) const;
    void logError(const std::string& message) const;
    
    std::string createOutputFileName(const std::string& base_name, 
                                   const std::string& extension,
                                   const AnalysisOptions& options) const;
    
    bool writeToFile(const std::string& content, 
                    const std::string& file_path) const;
    
    double getCurrentTimeMs() const;
};

} // namespace UnityContextGen