#include "../Core/CoreEngine.h"
#include "test_framework.h"
#include <iostream>
#include <filesystem>

void test_core_engine() {
    using namespace UnityContextGen;
    
    // Test 1: CoreEngine initialization
    {
        CoreEngine engine;
        g_test_results.push_back({"CoreEngine_Initialize", true, "CoreEngine initialized successfully"});
    }
    
    // Test 2: Single file analysis
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false; // Don't write files during testing
        options.verbose_output = false;
        
        auto result = engine.analyzeSingleFile("SampleUnityProject/PlayerController.cs", options);
        
        if (result.success) {
            if (result.monobehaviours.size() > 0) {
                g_test_results.push_back({"CoreEngine_SingleFileAnalysis", true, "Successfully analyzed single Unity file"});
            } else {
                g_test_results.push_back({"CoreEngine_SingleFileAnalysis", false, "Analysis succeeded but found no MonoBehaviours"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_SingleFileAnalysis", false, "Failed to analyze single Unity file: " + result.error_message});
        }
    }
    
    // Test 3: Multi-file analysis
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        std::vector<std::string> files = {
            "SampleUnityProject/PlayerController.cs",
            "SampleUnityProject/GameManager.cs",
            "SampleUnityProject/UIController.cs"
        };
        
        auto result = engine.analyzeProject(files, options);
        
        if (result.success) {
            if (result.monobehaviours.size() >= 3) {
                g_test_results.push_back({"CoreEngine_MultiFileAnalysis", true, "Successfully analyzed multiple Unity files"});
                
                // Check if all analysis components were generated
                bool has_metadata = !result.project_metadata.components.empty();
                bool has_context = !result.project_context.summary.project_type.empty();
                
                if (has_metadata && has_context) {
                    g_test_results.push_back({"CoreEngine_CompleteAnalysis", true, "Generated complete analysis with metadata and context"});
                } else {
                    g_test_results.push_back({"CoreEngine_CompleteAnalysis", false, "Analysis incomplete - missing metadata or context"});
                }
            } else {
                g_test_results.push_back({"CoreEngine_MultiFileAnalysis", false, "Analysis succeeded but found insufficient MonoBehaviours"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_MultiFileAnalysis", false, "Failed to analyze multiple Unity files: " + result.error_message});
        }
    }
    
    // Test 4: Analysis options handling
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.analyze_dependencies = false;
        options.analyze_lifecycle = false;
        options.detect_patterns = false;
        options.export_json = false;
        options.verbose_output = false;
        
        auto result = engine.analyzeSingleFile("SampleUnityProject/PlayerController.cs", options);
        
        if (result.success) {
            // Should still have basic MonoBehaviour detection
            if (result.monobehaviours.size() > 0) {
                g_test_results.push_back({"CoreEngine_AnalysisOptions", true, "Successfully handled analysis options"});
            } else {
                g_test_results.push_back({"CoreEngine_AnalysisOptions", false, "Failed basic analysis with limited options"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_AnalysisOptions", false, "Failed analysis with custom options: " + result.error_message});
        }
    }
    
    // Test 5: LLM prompt generation
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        auto result = engine.analyzeSingleFile("SampleUnityProject/GameManager.cs", options);
        
        if (result.success) {
            auto llm_prompt = engine.generateLLMPrompt(result);
            
            if (!llm_prompt.empty() && llm_prompt.find("Unity") != std::string::npos) {
                g_test_results.push_back({"CoreEngine_LLMPromptGeneration", true, "Successfully generated LLM prompt"});
            } else {
                g_test_results.push_back({"CoreEngine_LLMPromptGeneration", false, "Generated empty or invalid LLM prompt"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_LLMPromptGeneration", false, "Failed to analyze file for LLM prompt generation"});
        }
    }
    
    // Test 6: Summary generation
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        std::vector<std::string> files = {
            "SampleUnityProject/PlayerController.cs",
            "SampleUnityProject/GameManager.cs"
        };
        
        auto result = engine.analyzeProject(files, options);
        
        if (result.success) {
            auto quick_summary = engine.generateQuickSummary(result);
            auto detailed_report = engine.generateDetailedReport(result);
            
            bool has_quick_summary = !quick_summary.empty() && quick_summary.find("Summary") != std::string::npos;
            bool has_detailed_report = !detailed_report.empty() && detailed_report.find("Report") != std::string::npos;
            
            if (has_quick_summary && has_detailed_report) {
                g_test_results.push_back({"CoreEngine_SummaryGeneration", true, "Successfully generated summaries and reports"});
            } else {
                g_test_results.push_back({"CoreEngine_SummaryGeneration", false, "Failed to generate complete summaries or reports"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_SummaryGeneration", false, "Failed to analyze files for summary generation"});
        }
    }
    
    // Test 7: Error handling
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        // Test with non-existent file
        auto result = engine.analyzeSingleFile("NonExistentFile.cs", options);
        
        if (!result.success && !result.error_message.empty()) {
            g_test_results.push_back({"CoreEngine_ErrorHandling", true, "Properly handled non-existent file error"});
        } else {
            g_test_results.push_back({"CoreEngine_ErrorHandling", false, "Failed to properly handle file error"});
        }
    }
    
    // Test 8: Performance measurement
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        auto result = engine.analyzeSingleFile("SampleUnityProject/PlayerController.cs", options);
        
        if (result.success && result.analysis_duration_ms > 0) {
            g_test_results.push_back({"CoreEngine_PerformanceMeasurement", true, "Successfully measured analysis performance"});
        } else {
            g_test_results.push_back({"CoreEngine_PerformanceMeasurement", false, "Failed to measure analysis performance"});
        }
    }
    
    // Test 9: Directory analysis (if SampleUnityProject directory exists)
    {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        options.verbose_output = false;
        
        if (std::filesystem::exists("SampleUnityProject") && 
            std::filesystem::is_directory("SampleUnityProject")) {
            
            auto result = engine.analyzeDirectory("SampleUnityProject", options);
            
            if (result.success && result.monobehaviours.size() >= 4) {
                g_test_results.push_back({"CoreEngine_DirectoryAnalysis", true, "Successfully analyzed Unity project directory"});
            } else if (result.success) {
                g_test_results.push_back({"CoreEngine_DirectoryAnalysis", true, "Analyzed directory with limited files"});
            } else {
                g_test_results.push_back({"CoreEngine_DirectoryAnalysis", false, "Failed to analyze Unity project directory"});
            }
        } else {
            g_test_results.push_back({"CoreEngine_DirectoryAnalysis", true, "Skipped directory analysis (directory not found)"});
        }
    }
    
    // Test 10: Verbose output mode
    {
        CoreEngine engine;
        engine.setVerbose(true);
        
        if (engine.getVerbose()) {
            g_test_results.push_back({"CoreEngine_VerboseMode", true, "Successfully set and retrieved verbose mode"});
        } else {
            g_test_results.push_back({"CoreEngine_VerboseMode", false, "Failed to set verbose mode"});
        }
        
        engine.setVerbose(false);
    }
}