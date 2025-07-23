#include "CoreEngine.h"
#include "MetadataGenerator/JSONExporter.h"
#include "Configuration/AnalysisConfig.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace UnityContextGen {

CoreEngine::CoreEngine() : m_verbose(false) {
    // Load configuration
    Config::ConfigurationManager::getInstance().loadConfig();
    
    // Initialize Claude analyzer if API key is available
    auto& config = Config::ConfigurationManager::getInstance().getAIConfig();
    if (config.enable_ai_analysis && !config.api_key.empty()) {
        m_claude_analyzer = std::make_unique<AI::ClaudeAnalyzer>();
        if (m_claude_analyzer->initialize(config)) {
            logVerbose("Claude API analyzer initialized successfully");
        } else {
            logVerbose("Claude API analyzer initialization failed");
            m_claude_analyzer.reset();
        }
    }
}

AnalysisResult CoreEngine::analyzeProject(const std::vector<std::string>& source_files,
                                         const AnalysisOptions& options) const {
    auto start_time = getCurrentTimeMs();
    
    AnalysisResult result;
    result.success = false;
    
    logVerbose("Starting project analysis for " + std::to_string(source_files.size()) + " files");
    
    try {
        std::vector<Unity::MonoBehaviourInfo> all_monobehaviours;
        
        for (const auto& file_path : source_files) {
            if (!isValidCSharpFile(file_path)) {
                logVerbose("Skipping non-C# file: " + file_path);
                continue;
            }
            
            logVerbose("Analyzing file: " + file_path);
            
            if (!m_mb_analyzer.analyzeFile(file_path)) {
                logError("Failed to analyze file: " + file_path);
                continue;
            }
            
            auto file_monobehaviours = m_mb_analyzer.getMonoBehaviours();
            all_monobehaviours.insert(all_monobehaviours.end(),
                                    file_monobehaviours.begin(),
                                    file_monobehaviours.end());
        }
        
        result.monobehaviours = all_monobehaviours;
        logVerbose("Found " + std::to_string(all_monobehaviours.size()) + " MonoBehaviour components");
        
        if (options.analyze_dependencies) {
            logVerbose("Analyzing component dependencies");
            m_dep_analyzer.analyzeProject(all_monobehaviours);
            result.dependency_graph = m_dep_analyzer.buildDependencyGraph();
        }
        
        if (options.analyze_lifecycle) {
            logVerbose("Analyzing Unity lifecycle methods");
            m_lifecycle_analyzer.analyzeProject(all_monobehaviours);
            result.lifecycle_flows = m_lifecycle_analyzer.getAllLifecycleFlows();
        }
        
        if (options.detect_patterns) {
            logVerbose("Detecting Unity design patterns");
            m_pattern_detector.analyzeProject(all_monobehaviours, result.dependency_graph);
            result.patterns = m_pattern_detector.getDetectedPatterns();
        }
        
        if (options.analyze_scriptable_objects) {
            logVerbose("Analyzing ScriptableObjects");
            m_scriptable_object_analyzer.analyzeProject(source_files);
            result.scriptable_objects = m_scriptable_object_analyzer.getScriptableObjects();
            result.asset_dependencies = m_scriptable_object_analyzer.getAssetDependencies();
        }
        
        // AI-powered Unity API analysis
        logVerbose("Detecting Unity API usage");
        m_api_detector.setVerbose(m_verbose);
        result.api_usage = m_api_detector.analyzeProject(source_files);
        
        // Claude AI analysis (if available and enabled)
        if (m_claude_analyzer && Config::ConfigurationManager::getInstance().getAIConfig().enable_ai_analysis) {
            logVerbose("Running AI-powered code analysis");
            
            for (const auto& usage : result.api_usage) {
                if (!usage.detected_apis.empty()) {
                    AI::AIAnalysisRequest request;
                    request.component_name = usage.class_name;
                    request.file_path = usage.file_path;
                    request.analysis_type = "code_review";
                    request.project_context = "Unity game development project";
                    
                    // Read file content for Claude analysis
                    std::ifstream file(usage.file_path);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        request.code_content = buffer.str();
                        file.close();
                        
                        // Get AI analysis
                        auto ai_result = m_claude_analyzer->analyzeCode(request);
                        if (ai_result.success) {
                            result.ai_analysis_results.push_back(ai_result);
                            logVerbose("AI analysis completed for " + usage.class_name);
                        }
                    }
                }
            }
        }
        
        if (options.generate_metadata) {
            logVerbose("Generating project metadata");
            m_metadata_generator.analyzeProject(all_monobehaviours,
                                              result.dependency_graph,
                                              result.lifecycle_flows,
                                              result.patterns,
                                              source_files);
            result.project_metadata = m_metadata_generator.getProjectMetadata();
        }
        
        if (options.generate_context) {
            logVerbose("Generating project context summary");
            result.project_context = m_context_summarizer.generateProjectContext(result.project_metadata);
        }
        
        result.success = true;
        result.analysis_duration_ms = getCurrentTimeMs() - start_time;
        
        if (options.export_json) {
            exportResults(result, options);
        }
        
        logVerbose("Analysis completed successfully in " + 
                  std::to_string(result.analysis_duration_ms) + "ms");
        
    } catch (const std::exception& e) {
        result.error_message = "Analysis failed: " + std::string(e.what());
        logError(result.error_message);
    }
    
    return result;
}

AnalysisResult CoreEngine::analyzeDirectory(const std::string& directory_path,
                                           const AnalysisOptions& options) const {
    logVerbose("Scanning directory: " + directory_path);
    
    auto source_files = findCSharpFiles(directory_path);
    if (source_files.empty()) {
        AnalysisResult result;
        result.success = false;
        result.error_message = "No C# files found in directory: " + directory_path;
        logError(result.error_message);
        return result;
    }
    
    logVerbose("Found " + std::to_string(source_files.size()) + " C# files");
    return analyzeProject(source_files, options);
}

AnalysisResult CoreEngine::analyzeSingleFile(const std::string& file_path,
                                            const AnalysisOptions& options) const {
    return analyzeProject({file_path}, options);
}

bool CoreEngine::exportResults(const AnalysisResult& result, const AnalysisOptions& options) const {
    if (!result.success) {
        logError("Cannot export failed analysis results");
        return false;
    }
    
    try {
        std::filesystem::create_directories(options.output_directory);
        
        Metadata::JSONExporter exporter;
        
        std::string project_file = createOutputFileName("project_metadata", "json", options);
        if (exporter.exportProjectMetadata(result.project_metadata, project_file)) {
            logVerbose("Exported project metadata to: " + project_file);
        }
        
        std::string context_file = createOutputFileName("project_context", "json", options);
        auto context_json = m_context_summarizer.exportContextToJSON(result.project_context);
        if (exporter.writeToFile(context_json, context_file)) {
            logVerbose("Exported project context to: " + context_file);
        }
        
        std::string llm_file = createOutputFileName("llm_optimized", "json", options);
        auto llm_json = m_context_summarizer.exportLLMOptimizedJSON(result.project_context);
        if (exporter.writeToFile(llm_json, llm_file)) {
            logVerbose("Exported LLM-optimized data to: " + llm_file);
        }
        
        std::string summary_file = createOutputFileName("summary", "md", options);
        std::string summary_content = generateQuickSummary(result);
        if (writeToFile(summary_content, summary_file)) {
            logVerbose("Exported summary to: " + summary_file);
        }
        
        std::string report_file = createOutputFileName("detailed_report", "md", options);
        std::string report_content = generateDetailedReport(result);
        if (writeToFile(report_content, report_file)) {
            logVerbose("Exported detailed report to: " + report_file);
        }
        
        std::string prompt_file = createOutputFileName("llm_prompt", "md", options);
        std::string prompt_content = generateLLMPrompt(result);
        if (writeToFile(prompt_content, prompt_file)) {
            logVerbose("Exported LLM prompt to: " + prompt_file);
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Failed to export results: " + std::string(e.what()));
        return false;
    }
}

std::string CoreEngine::generateLLMPrompt(const AnalysisResult& result) const {
    if (!result.success) {
        return "Analysis failed: " + result.error_message;
    }
    
    return m_context_summarizer.generateLLMContextPrompt(result.project_context);
}

std::string CoreEngine::generateQuickSummary(const AnalysisResult& result) const {
    if (!result.success) {
        return "Analysis failed: " + result.error_message;
    }
    
    std::stringstream summary;
    
    summary << "# Unity Project Analysis Summary\n\n";
    
    summary << "**Analysis Date:** " << result.project_metadata.analysis_timestamp << "\n";
    summary << "**Analysis Duration:** " << result.analysis_duration_ms << "ms\n\n";
    
    summary << "## Project Overview\n";
    summary << "- **Type:** " << result.project_context.summary.project_type << "\n";
    summary << "- **Architecture:** " << result.project_context.summary.architecture_pattern << "\n";
    summary << "- **Complexity:** " << result.project_context.summary.complexity_level << "\n";
    summary << "- **Quality Score:** " << static_cast<int>(result.project_context.summary.quality_score) << "%\n\n";
    
    summary << "## Components Found\n";
    summary << "- **Total MonoBehaviours:** " << result.monobehaviours.size() << "\n";
    summary << "- **Total Dependencies:** " << result.project_metadata.project_summary.total_dependencies << "\n";
    summary << "- **System Groups:** " << result.project_metadata.systems.system_groups.size() << "\n\n";
    
    if (!result.project_context.critical_components.empty()) {
        summary << "## Critical Components\n";
        for (const auto& component : result.project_context.critical_components) {
            summary << "- " << component << "\n";
        }
        summary << "\n";
    }
    
    if (!result.project_context.improvement_priorities.empty()) {
        summary << "## Top Improvement Priorities\n";
        for (size_t i = 0; i < std::min(static_cast<size_t>(3), result.project_context.improvement_priorities.size()); ++i) {
            summary << (i + 1) << ". " << result.project_context.improvement_priorities[i] << "\n";
        }
    }
    
    return summary.str();
}

std::string CoreEngine::generateDetailedReport(const AnalysisResult& result) const {
    if (!result.success) {
        return "Analysis failed: " + result.error_message;
    }
    
    std::stringstream report;
    
    report << "# Detailed Unity Project Analysis Report\n\n";
    
    report << "**Generated:** " << result.project_metadata.analysis_timestamp << "\n";
    report << "**Analysis Time:** " << result.analysis_duration_ms << "ms\n\n";
    
    report << m_context_summarizer.generateArchitectureOverview(result.project_context);
    report << "\n";
    
    report << "## Component Analysis\n";
    report << "Total Components: " << result.monobehaviours.size() << "\n\n";
    
    for (const auto& mb_info : result.monobehaviours) {
        report << "### " << mb_info.class_name << "\n";
        report << "- **File:** " << mb_info.file_path << "\n";
        
        auto comp_it = result.project_metadata.components.find(mb_info.class_name);
        if (comp_it != result.project_metadata.components.end()) {
            report << "- **Purpose:** " << comp_it->second.purpose << "\n";
            
            if (!comp_it->second.dependencies.empty()) {
                report << "- **Dependencies:** ";
                for (size_t i = 0; i < comp_it->second.dependencies.size(); ++i) {
                    if (i > 0) report << ", ";
                    report << comp_it->second.dependencies[i];
                }
                report << "\n";
            }
            
            report << "- **Complexity Score:** " << comp_it->second.metrics.complexity_score << "\n";
        }
        
        if (!mb_info.unity_methods.empty()) {
            report << "- **Unity Methods:** ";
            for (size_t i = 0; i < mb_info.unity_methods.size(); ++i) {
                if (i > 0) report << ", ";
                report << mb_info.unity_methods[i];
            }
            report << "\n";
        }
        
        report << "\n";
    }
    
    if (!result.patterns.empty()) {
        report << "## Design Patterns Detected\n";
        for (const auto& pattern : result.patterns) {
            report << "### " << pattern.pattern_name << "\n";
            report << "- **Confidence:** " << static_cast<int>(pattern.confidence_score * 100) << "%\n";
            report << "- **Components:** ";
            for (size_t i = 0; i < pattern.involved_components.size(); ++i) {
                if (i > 0) report << ", ";
                report << pattern.involved_components[i];
            }
            report << "\n";
            report << "- **Purpose:** " << pattern.purpose << "\n\n";
        }
    }
    
    if (result.dependency_graph.dependencies.size() > 0) {
        report << "## Dependency Analysis\n";
        
        if (result.project_metadata.dependencies.has_circular_dependencies) {
            report << "⚠️ **Warning:** Circular dependencies detected!\n\n";
        }
        
        if (!result.project_metadata.dependencies.high_coupling_components.empty()) {
            report << "**High Coupling Components:**\n";
            for (const auto& component : result.project_metadata.dependencies.high_coupling_components) {
                report << "- " << component << "\n";
            }
            report << "\n";
        }
        
        if (!result.project_metadata.dependencies.isolated_components.empty()) {
            report << "**Isolated Components:**\n";
            for (const auto& component : result.project_metadata.dependencies.isolated_components) {
                report << "- " << component << "\n";
            }
            report << "\n";
        }
    }
    
    report << "## Recommendations\n";
    if (!result.project_metadata.quality.improvement_suggestions.empty()) {
        for (const auto& suggestion : result.project_metadata.quality.improvement_suggestions) {
            report << "- " << suggestion << "\n";
        }
    } else {
        report << "No specific improvements identified. Project structure looks good!\n";
    }
    
    return report.str();
}

std::vector<std::string> CoreEngine::findCSharpFiles(const std::string& directory_path) const {
    std::vector<std::string> cs_files;
    
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cs") {
                cs_files.push_back(entry.path().string());
            }
        }
    } catch (const std::exception& e) {
        logError("Error scanning directory: " + std::string(e.what()));
    }
    
    return cs_files;
}

bool CoreEngine::isValidCSharpFile(const std::string& file_path) const {
    // C++17 compatible string suffix check
    size_t len = file_path.length();
    return len >= 3 && file_path.substr(len - 3) == ".cs" && std::filesystem::exists(file_path);
}

void CoreEngine::logVerbose(const std::string& message) const {
    if (m_verbose) {
        std::cout << "[VERBOSE] " << message << std::endl;
    }
}

void CoreEngine::logError(const std::string& message) const {
    std::cerr << "[ERROR] " << message << std::endl;
}

std::string CoreEngine::createOutputFileName(const std::string& base_name,
                                            const std::string& extension,
                                            const AnalysisOptions& options) const {
    std::filesystem::path output_path(options.output_directory);
    output_path /= (base_name + "." + extension);
    return output_path.string();
}

bool CoreEngine::writeToFile(const std::string& content, const std::string& file_path) const {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            logError("Failed to open file for writing: " + file_path);
            return false;
        }
        
        file << content;
        file.close();
        
        return true;
    } catch (const std::exception& e) {
        logError("Error writing to file " + file_path + ": " + std::string(e.what()));
        return false;
    }
}

double CoreEngine::getCurrentTimeMs() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

} // namespace UnityContextGen