#include "../Core/MetadataGenerator/ProjectMetadata.h"
#include "../Core/MetadataGenerator/JSONExporter.h"
#include "../Core/UnityAnalyzer/MonoBehaviourAnalyzer.h"
#include "../Core/UnityAnalyzer/ComponentDependencyAnalyzer.h"
#include "../Core/UnityAnalyzer/LifecycleAnalyzer.h"
#include "../Core/UnityAnalyzer/UnityPatternDetector.h"
#include "test_framework.h"
#include <iostream>
#include <fstream>

void test_metadata_generator() {
    using namespace UnityContextGen::Metadata;
    using namespace UnityContextGen::Unity;
    
    // Set up analyzers
    MonoBehaviourAnalyzer mb_analyzer;
    ComponentDependencyAnalyzer dep_analyzer;
    LifecycleAnalyzer lifecycle_analyzer;
    UnityPatternDetector pattern_detector;
    
    // Analyze sample files
    std::vector<std::string> files = {
        "SampleUnityProject/PlayerController.cs",
        "SampleUnityProject/GameManager.cs",
        "SampleUnityProject/UIController.cs"
    };
    
    std::vector<MonoBehaviourInfo> all_monobehaviours;
    
    for (const auto& file : files) {
        bool success = mb_analyzer.analyzeFile(file);
        if (success) {
            auto file_monobehaviours = mb_analyzer.getMonoBehaviours();
            all_monobehaviours.insert(all_monobehaviours.end(),
                                    file_monobehaviours.begin(),
                                    file_monobehaviours.end());
        }
    }
    
    if (all_monobehaviours.empty()) {
        g_test_results.push_back({"MetadataGenerator_Setup", false, "Failed to analyze sample Unity files"});
        return;
    }
    
    // Analyze dependencies and patterns
    dep_analyzer.analyzeProject(all_monobehaviours);
    auto dependency_graph = dep_analyzer.buildDependencyGraph();
    
    lifecycle_analyzer.analyzeProject(all_monobehaviours);
    auto lifecycle_flows = lifecycle_analyzer.getAllLifecycleFlows();
    
    pattern_detector.analyzeProject(all_monobehaviours, dependency_graph);
    auto patterns = pattern_detector.getDetectedPatterns();
    
    // Test 1: Component metadata generation
    {
        ComponentMetadataGenerator comp_generator;
        comp_generator.analyzeProject(all_monobehaviours, dependency_graph, lifecycle_flows, patterns);
        
        auto component_metadata = comp_generator.getAllComponentMetadata();
        
        if (component_metadata.size() >= 3) {
            bool foundPlayerController = false;
            bool foundGameManager = false;
            
            for (const auto& comp : component_metadata) {
                if (comp.class_name == "PlayerController") {
                    foundPlayerController = true;
                    
                    // Check if purpose was inferred
                    if (!comp.purpose.empty()) {
                        g_test_results.push_back({"ComponentMetadataGenerator_InferPurpose", true, "Successfully inferred component purpose"});
                    } else {
                        g_test_results.push_back({"ComponentMetadataGenerator_InferPurpose", false, "Failed to infer component purpose"});
                    }
                    
                    // Check if lifecycle info was populated
                    if (!comp.lifecycle.methods.empty()) {
                        g_test_results.push_back({"ComponentMetadataGenerator_LifecycleInfo", true, "Successfully populated lifecycle information"});
                    } else {
                        g_test_results.push_back({"ComponentMetadataGenerator_LifecycleInfo", false, "Failed to populate lifecycle information"});
                    }
                    
                    // Check if responsibility blocks were generated
                    if (!comp.responsibility_blocks.blocks.empty()) {
                        g_test_results.push_back({"ComponentMetadataGenerator_ResponsibilityBlocks", true, "Successfully generated responsibility blocks"});
                    } else {
                        g_test_results.push_back({"ComponentMetadataGenerator_ResponsibilityBlocks", false, "Failed to generate responsibility blocks"});
                    }
                }
                
                if (comp.class_name == "GameManager") {
                    foundGameManager = true;
                }
            }
            
            if (foundPlayerController && foundGameManager) {
                g_test_results.push_back({"ComponentMetadataGenerator_GenerateMetadata", true, "Successfully generated component metadata"});
            } else {
                g_test_results.push_back({"ComponentMetadataGenerator_GenerateMetadata", false, "Failed to find expected components in metadata"});
            }
        } else {
            g_test_results.push_back({"ComponentMetadataGenerator_AnalyzeProject", false, "Failed to generate sufficient component metadata"});
        }
    }
    
    // Test 2: Project metadata generation
    {
        ProjectMetadataGenerator project_generator;
        project_generator.analyzeProject(all_monobehaviours, dependency_graph, lifecycle_flows, patterns, files);
        
        auto project_metadata = project_generator.getProjectMetadata();
        
        // Check project summary
        if (!project_metadata.project_summary.game_type.empty() &&
            !project_metadata.project_summary.architecture_pattern.empty()) {
            g_test_results.push_back({"ProjectMetadataGenerator_ProjectSummary", true, "Successfully generated project summary"});
        } else {
            g_test_results.push_back({"ProjectMetadataGenerator_ProjectSummary", false, "Failed to generate complete project summary"});
        }
        
        // Check system analysis
        if (project_metadata.systems.system_groups.size() > 0) {
            g_test_results.push_back({"ProjectMetadataGenerator_SystemAnalysis", true, "Successfully analyzed system groups"});
        } else {
            g_test_results.push_back({"ProjectMetadataGenerator_SystemAnalysis", false, "Failed to analyze system groups"});
        }
        
        // Check quality metrics
        if (project_metadata.quality.maintainability_score > 0) {
            g_test_results.push_back({"ProjectMetadataGenerator_QualityMetrics", true, "Successfully calculated quality metrics"});
        } else {
            g_test_results.push_back({"ProjectMetadataGenerator_QualityMetrics", false, "Failed to calculate quality metrics"});
        }
        
        // Check if components are included
        if (project_metadata.components.size() >= 3) {
            g_test_results.push_back({"ProjectMetadataGenerator_IncludeComponents", true, "Successfully included component metadata"});
        } else {
            g_test_results.push_back({"ProjectMetadataGenerator_IncludeComponents", false, "Failed to include component metadata"});
        }
    }
    
    // Test 3: JSON export functionality
    {
        ProjectMetadataGenerator project_generator;
        project_generator.analyzeProject(all_monobehaviours, dependency_graph, lifecycle_flows, patterns, files);
        
        auto project_metadata = project_generator.getProjectMetadata();
        
        JSONExporter exporter;
        
        // Test project JSON export
        auto project_json = project_generator.exportToJSON();
        if (!project_json.empty() && project_json.contains("project_context")) {
            g_test_results.push_back({"JSONExporter_ProjectAnalysis", true, "Successfully exported project analysis to JSON"});
        } else {
            g_test_results.push_back({"JSONExporter_ProjectAnalysis", false, "Failed to export project analysis to JSON"});
        }
        
        // Test LLM optimized JSON export
        auto llm_json = exporter.createClaudeCodeOptimizedJSON(project_metadata);
        if (!llm_json.empty() && llm_json.contains("project_context")) {
            g_test_results.push_back({"JSONExporter_ClaudeCodeOptimized", true, "Successfully exported Claude Code optimized JSON"});
        } else {
            g_test_results.push_back({"JSONExporter_ClaudeCodeOptimized", false, "Failed to export Claude Code optimized JSON"});
        }
        
        // Test architecture overview
        auto architecture_json = exporter.createArchitectureOverviewJSON(project_metadata);
        if (!architecture_json.empty() && architecture_json.contains("architecture_summary")) {
            g_test_results.push_back({"JSONExporter_ArchitectureOverview", true, "Successfully exported architecture overview"});
        } else {
            g_test_results.push_back({"JSONExporter_ArchitectureOverview", false, "Failed to export architecture overview"});
        }
    }
    
    // Test 4: LLM prompt formatting
    {
        ProjectMetadataGenerator project_generator;
        project_generator.analyzeProject(all_monobehaviours, dependency_graph, lifecycle_flows, patterns, files);
        
        auto project_metadata = project_generator.getProjectMetadata();
        
        JSONExporter exporter;
        
        // Test LLM prompt generation
        auto llm_prompt = exporter.formatForLLMPrompt(project_metadata);
        if (!llm_prompt.empty() && llm_prompt.find("Project Context:") != std::string::npos) {
            g_test_results.push_back({"JSONExporter_LLMPrompt", true, "Successfully generated LLM-friendly prompt"});
        } else {
            g_test_results.push_back({"JSONExporter_LLMPrompt", false, "Failed to generate LLM-friendly prompt"});
        }
        
        // Test component-specific prompt
        if (!project_metadata.components.empty()) {
            auto comp_name = project_metadata.components.begin()->first;
            auto comp_metadata = project_metadata.components.begin()->second;
            
            auto comp_prompt = exporter.formatComponentForLLMPrompt(comp_metadata);
            if (!comp_prompt.empty() && comp_prompt.find(comp_name) != std::string::npos) {
                g_test_results.push_back({"JSONExporter_ComponentPrompt", true, "Successfully generated component-specific prompt"});
            } else {
                g_test_results.push_back({"JSONExporter_ComponentPrompt", false, "Failed to generate component-specific prompt"});
            }
        }
    }
    
    // Test 5: File writing
    {
        ProjectMetadataGenerator project_generator;
        project_generator.analyzeProject(all_monobehaviours, dependency_graph, lifecycle_flows, patterns, files);
        
        auto project_metadata = project_generator.getProjectMetadata();
        
        JSONExporter exporter;
        
        // Test writing to file
        std::string test_file = "test_output.json";
        auto test_json = exporter.createClaudeCodeOptimizedJSON(project_metadata);
        
        bool write_success = exporter.writeToFile(test_json, test_file);
        if (write_success) {
            // Check if file was actually created
            std::ifstream check_file(test_file);
            if (check_file.good()) {
                g_test_results.push_back({"JSONExporter_WriteToFile", true, "Successfully wrote JSON to file"});
                check_file.close();
                
                // Clean up test file
                std::remove(test_file.c_str());
            } else {
                g_test_results.push_back({"JSONExporter_WriteToFile", false, "File was not created successfully"});
            }
        } else {
            g_test_results.push_back({"JSONExporter_WriteToFile", false, "Failed to write JSON to file"});
        }
    }
    
    g_test_results.push_back({"MetadataGenerator_TestSuite", true, "Metadata generator test suite completed"});
}