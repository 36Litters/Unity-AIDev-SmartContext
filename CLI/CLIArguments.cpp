#include "CLIArguments.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace UnityContextGen {
namespace CLI {

CLIArguments::CLIArguments() {
}

bool CLIArguments::parseArguments(int argc, char* argv[]) {
    if (argc < 2) {
        m_options.help = true;
        return true;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            m_options.help = true;
            return true;
        } else if (arg == "--version" || arg == "-v") {
            m_options.version = true;
            return true;
        } else if (arg == "--verbose") {
            m_options.verbose = true;
        } else if (arg == "--quiet" || arg == "-q") {
            m_options.quiet = true;
        } else if (arg == "--force") {
            m_options.force_overwrite = true;
        } else if (arg == "--no-dependencies") {
            m_options.analyze_dependencies = false;
        } else if (arg == "--no-lifecycle") {
            m_options.analyze_lifecycle = false;
        } else if (arg == "--no-patterns") {
            m_options.detect_patterns = false;
        } else if (arg == "--no-metadata") {
            m_options.generate_metadata = false;
        } else if (arg == "--no-context") {
            m_options.generate_context = false;
        } else if (arg == "--no-json") {
            m_options.export_json = false;
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) {
                m_options.output_directory = argv[++i];
            } else {
                m_error_message = "Missing value for " + arg;
                return false;
            }
        } else if (arg == "--directory" || arg == "-d") {
            if (i + 1 < argc) {
                m_options.input_directory = argv[++i];
            } else {
                m_error_message = "Missing value for " + arg;
                return false;
            }
        } else if (arg == "--format" || arg == "-f") {
            if (i + 1 < argc) {
                std::string format = argv[++i];
                std::transform(format.begin(), format.end(), format.begin(), ::tolower);
                
                if (format == "json") {
                    m_options.output_format = CLIOptions::OutputFormat::JSON;
                } else if (format == "markdown" || format == "md") {
                    m_options.output_format = CLIOptions::OutputFormat::Markdown;
                } else if (format == "llm" || format == "prompt") {
                    m_options.output_format = CLIOptions::OutputFormat::LLMPrompt;
                } else if (format == "all") {
                    m_options.output_format = CLIOptions::OutputFormat::All;
                } else {
                    m_error_message = "Invalid output format: " + format;
                    return false;
                }
            } else {
                m_error_message = "Missing value for " + arg;
                return false;
            }
        } else if (arg.length() > 0 && arg[0] == '-') {
            m_error_message = "Unknown option: " + arg;
            return false;
        } else {
            m_options.input_files.push_back(arg);
        }
    }
    
    return validateOptions();
}

bool CLIArguments::validateOptions() {
    if (m_options.help || m_options.version) {
        return true;
    }
    
    if (m_options.input_files.empty() && m_options.input_directory.empty()) {
        m_error_message = "No input files or directory specified";
        return false;
    }
    
    if (!m_options.input_files.empty() && !m_options.input_directory.empty()) {
        m_error_message = "Cannot specify both input files and input directory";
        return false;
    }
    
    if (!m_options.input_directory.empty()) {
        if (!std::filesystem::exists(m_options.input_directory)) {
            m_error_message = "Input directory does not exist: " + m_options.input_directory;
            return false;
        }
        
        if (!std::filesystem::is_directory(m_options.input_directory)) {
            m_error_message = "Input path is not a directory: " + m_options.input_directory;
            return false;
        }
    }
    
    for (const auto& file : m_options.input_files) {
        if (!std::filesystem::exists(file)) {
            m_error_message = "Input file does not exist: " + file;
            return false;
        }
        
        size_t len = file.length();
        if (len < 3 || file.substr(len - 3) != ".cs") {
            m_error_message = "Input file is not a C# source file: " + file;
            return false;
        }
    }
    
    if (m_options.verbose && m_options.quiet) {
        m_error_message = "Cannot specify both --verbose and --quiet";
        return false;
    }
    
    setDefaultOutputDirectory();
    
    return true;
}

void CLIArguments::setDefaultOutputDirectory() {
    if (m_options.output_directory == "./output") {
        if (!m_options.input_directory.empty()) {
            std::filesystem::path input_path(m_options.input_directory);
            std::filesystem::path output_path = input_path / "unity_context_analysis";
            m_options.output_directory = output_path.string();
        } else if (!m_options.input_files.empty()) {
            std::filesystem::path first_file(m_options.input_files[0]);
            std::filesystem::path output_path = first_file.parent_path() / "unity_context_analysis";
            m_options.output_directory = output_path.string();
        }
    }
}

bool CLIArguments::isValid() const {
    return m_error_message.empty();
}

std::string CLIArguments::getHelpText() const {
    return R"(Unity LLM Context Generator

USAGE:
    unity_context_generator [OPTIONS] [FILES...]
    unity_context_generator --directory <DIR> [OPTIONS]

DESCRIPTION:
    Analyzes Unity C# scripts to generate LLM-friendly context metadata.
    Extracts component relationships, Unity lifecycle patterns, and architecture insights.

OPTIONS:
    -h, --help              Show this help message
    -v, --version           Show version information
    -d, --directory <DIR>   Analyze all C# files in directory recursively
    -o, --output <DIR>      Output directory (default: ./unity_context_analysis)
    -f, --format <FORMAT>   Output format: json, markdown, llm, all (default: all)
    
    --verbose               Enable verbose output
    -q, --quiet             Suppress all output except errors
    --force                 Overwrite existing output files
    
    --no-dependencies       Skip dependency analysis
    --no-lifecycle          Skip Unity lifecycle analysis
    --no-patterns           Skip design pattern detection
    --no-metadata           Skip metadata generation
    --no-context            Skip context summarization
    --no-json               Skip JSON export

EXAMPLES:
    # Analyze a single file
    unity_context_generator PlayerController.cs
    
    # Analyze entire Unity project
    unity_context_generator --directory ./Assets/Scripts
    
    # Generate only LLM prompt
    unity_context_generator -d ./Scripts -f llm -o ./llm_context
    
    # Verbose analysis with custom output
    unity_context_generator --verbose -d ./Assets -o ./analysis

OUTPUT FILES:
    project_metadata.json   - Detailed component metadata
    project_context.json    - Architecture and design insights
    llm_optimized.json      - LLM-friendly context data
    summary.md              - Quick project overview
    detailed_report.md      - Comprehensive analysis report
    llm_prompt.md           - Ready-to-use LLM context prompt

For more information, visit: https://github.com/your-repo/unity-llm-context-generator
)";
}

std::string CLIArguments::getVersionText() const {
    return R"(Unity LLM Context Generator v1.0.0
Built with Tree-sitter C# parser
Designed for Claude Code and other LLM tools

Copyright (c) 2024 Unity Context Generator Project
Licensed under MIT License
)";
}

} // namespace CLI
} // namespace UnityContextGen