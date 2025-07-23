#pragma once

#include "../Core/CoreEngine.h"
#include <string>
#include <vector>

namespace UnityContextGen {
namespace CLI {

struct CLIOptions {
    std::vector<std::string> input_files;
    std::string input_directory;
    std::string output_directory = "./output";
    
    bool analyze_dependencies = true;
    bool analyze_lifecycle = true;
    bool detect_patterns = true;
    bool generate_metadata = true;
    bool generate_context = true;
    bool export_json = true;
    
    bool verbose = false;
    bool help = false;
    bool version = false;
    
    enum class OutputFormat {
        JSON,
        Markdown,
        LLMPrompt,
        All
    } output_format = OutputFormat::All;
    
    bool quiet = false;
    bool force_overwrite = false;
};

class CLIArguments {
public:
    CLIArguments();
    ~CLIArguments() = default;

    bool parseArguments(int argc, char* argv[]);
    CLIOptions getOptions() const { return m_options; }
    
    std::string getHelpText() const;
    std::string getVersionText() const;
    
    bool isValid() const;
    std::string getErrorMessage() const { return m_error_message; }

private:
    CLIOptions m_options;
    std::string m_error_message;
    
    bool parseFlag(const std::string& arg);
    bool parseOption(const std::string& arg, const std::string& value);
    bool validateOptions();
    
    void printUsage() const;
    void setDefaultOutputDirectory();
};

} // namespace CLI
} // namespace UnityContextGen