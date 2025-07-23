#include "CLIApplication.h"
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace UnityContextGen {
namespace CLI {

CLIApplication::CLIApplication() : m_engine(std::make_unique<CoreEngine>()) {
}

int CLIApplication::run(int argc, char* argv[]) {
    if (!m_args.parseArguments(argc, argv)) {
        printError("Error: " + m_args.getErrorMessage(), CLIOptions{});
        std::cout << "\nUse --help for usage information.\n";
        return 1;
    }
    
    auto options = m_args.getOptions();
    
    if (options.help) {
        std::cout << m_args.getHelpText() << std::endl;
        return 0;
    }
    
    if (options.version) {
        std::cout << m_args.getVersionText() << std::endl;
        return 0;
    }
    
    if (!m_args.isValid()) {
        printError("Error: " + m_args.getErrorMessage(), options);
        return 1;
    }
    
    return runAnalysis(options);
}

int CLIApplication::runAnalysis(const CLIOptions& options) {
    try {
        m_engine->setVerbose(options.verbose);
        
        if (!createOutputDirectory(options.output_directory)) {
            printError("Failed to create output directory: " + options.output_directory, options);
            return 1;
        }
        
        printProgress("Starting Unity project analysis...", options);
        
        AnalysisResult result;
        auto analysis_options = convertCLIOptions(options);
        
        if (!options.input_directory.empty()) {
            printProgress("Analyzing directory: " + options.input_directory, options);
            result = m_engine->analyzeDirectory(options.input_directory, analysis_options);
        } else {
            printProgress("Analyzing " + std::to_string(options.input_files.size()) + " files", options);
            result = m_engine->analyzeProject(options.input_files, analysis_options);
        }
        
        if (!result.success) {
            printError("Analysis failed: " + result.error_message, options);
            return 1;
        }
        
        printResults(result, options);
        
        if (!options.quiet) {
            std::cout << "\n✅ Analysis completed successfully!\n";
            std::cout << "📁 Results saved to: " << options.output_directory << "\n";
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        printError("Unexpected error: " + std::string(e.what()), options);
        return 1;
    }
}

void CLIApplication::printResults(const AnalysisResult& result, const CLIOptions& options) {
    if (options.quiet) return;
    
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "Unity Project Analysis Results\n";
    std::cout << std::string(50, '=') << "\n";
    
    std::cout << "📊 Analysis Duration: " << formatDuration(result.analysis_duration_ms) << "\n";
    std::cout << "🎮 Project Type: " << result.project_context.summary.project_type << "\n";
    std::cout << "🏗️  Architecture: " << result.project_context.summary.architecture_pattern << "\n";
    std::cout << "📈 Quality Score: " << static_cast<int>(result.project_context.summary.quality_score) << "%\n";
    
    printComponentSummary(result);
    printQualityMetrics(result);
    
    if (!result.project_context.critical_components.empty()) {
        std::cout << "\n⚠️  Critical Components:\n";
        for (const auto& component : result.project_context.critical_components) {
            std::cout << "   • " << component << "\n";
        }
    }
    
    if (!result.project_context.improvement_priorities.empty()) {
        std::cout << "\n🎯 Top Improvement Priorities:\n";
        for (size_t i = 0; i < std::min(static_cast<size_t>(3), result.project_context.improvement_priorities.size()); ++i) {
            std::cout << "   " << (i + 1) << ". " << result.project_context.improvement_priorities[i] << "\n";
        }
    }
    
    if (!result.output_files.empty()) {
        std::cout << "\n📄 Generated Files:\n";
        printOutputFiles(result.output_files);
    }
}

void CLIApplication::printComponentSummary(const AnalysisResult& result) {
    std::cout << "\n📦 Component Summary:\n";
    std::cout << "   • MonoBehaviours: " << result.monobehaviours.size() << "\n";
    std::cout << "   • System Groups: " << result.project_metadata.systems.system_groups.size() << "\n";
    std::cout << "   • Dependencies: " << result.project_metadata.project_summary.total_dependencies << "\n";
    
    if (!result.patterns.empty()) {
        std::cout << "   • Design Patterns: " << result.patterns.size() << "\n";
    }
    
    if (result.project_metadata.dependencies.has_circular_dependencies) {
        std::cout << "   ⚠️  Circular Dependencies: Yes\n";
    }
}

void CLIApplication::printQualityMetrics(const AnalysisResult& result) {
    std::cout << "\n📏 Quality Metrics:\n";
    std::cout << "   • Maintainability: " << static_cast<int>(result.project_metadata.quality.maintainability_score) << "%\n";
    std::cout << "   • Testability: " << static_cast<int>(result.project_metadata.quality.testability_score) << "%\n";
    std::cout << "   • Performance: " << static_cast<int>(result.project_metadata.quality.performance_score) << "%\n";
    std::cout << "   • Architecture: " << static_cast<int>(result.project_metadata.quality.architecture_score) << "%\n";
}

void CLIApplication::printOutputFiles(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        std::filesystem::path file_path(file);
        std::string filename = file_path.filename().string();
        
        try {
            size_t file_size = std::filesystem::file_size(file_path);
            std::cout << "   • " << filename << " (" << formatFileSize(file_size) << ")\n";
        } catch (const std::exception&) {
            std::cout << "   • " << filename << "\n";
        }
    }
}

void CLIApplication::printProgress(const std::string& message, const CLIOptions& options) {
    if (!options.quiet) {
        std::cout << message << std::endl;
    }
}

void CLIApplication::printError(const std::string& message, const CLIOptions& options) {
    std::cerr << message << std::endl;
}

AnalysisOptions CLIApplication::convertCLIOptions(const CLIOptions& cli_options) {
    AnalysisOptions analysis_options;
    
    analysis_options.analyze_dependencies = cli_options.analyze_dependencies;
    analysis_options.analyze_lifecycle = cli_options.analyze_lifecycle;
    analysis_options.detect_patterns = cli_options.detect_patterns;
    analysis_options.generate_metadata = cli_options.generate_metadata;
    analysis_options.generate_context = cli_options.generate_context;
    analysis_options.export_json = cli_options.export_json;
    analysis_options.output_directory = cli_options.output_directory;
    analysis_options.verbose_output = cli_options.verbose;
    
    return analysis_options;
}

bool CLIApplication::createOutputDirectory(const std::string& path) {
    try {
        std::filesystem::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string CLIApplication::formatDuration(double milliseconds) {
    if (milliseconds < 1000) {
        return std::to_string(static_cast<int>(milliseconds)) + "ms";
    } else {
        double seconds = milliseconds / 1000.0;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << seconds << "s";
        return oss.str();
    }
}

std::string CLIApplication::formatFileSize(size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    double size = static_cast<double>(bytes);
    int unit_index = 0;
    
    while (size >= 1024 && unit_index < 3) {
        size /= 1024;
        unit_index++;
    }
    
    std::ostringstream oss;
    if (unit_index == 0) {
        oss << static_cast<int>(size) << units[unit_index];
    } else {
        oss << std::fixed << std::setprecision(1) << size << units[unit_index];
    }
    
    return oss.str();
}

} // namespace CLI
} // namespace UnityContextGen