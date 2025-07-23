#pragma once

#include "CLIArguments.h"
#include "../Core/CoreEngine.h"
#include <memory>

namespace UnityContextGen {
namespace CLI {

class CLIApplication {
public:
    CLIApplication();
    ~CLIApplication() = default;

    int run(int argc, char* argv[]);

private:
    std::unique_ptr<CoreEngine> m_engine;
    CLIArguments m_args;
    
    int runAnalysis(const CLIOptions& options);
    void printResults(const AnalysisResult& result, const CLIOptions& options);
    void printProgress(const std::string& message, const CLIOptions& options);
    void printError(const std::string& message, const CLIOptions& options);
    
    AnalysisOptions convertCLIOptions(const CLIOptions& cli_options);
    
    bool createOutputDirectory(const std::string& path);
    std::string formatDuration(double milliseconds);
    std::string formatFileSize(size_t bytes);
    
    void printComponentSummary(const AnalysisResult& result);
    void printQualityMetrics(const AnalysisResult& result);
    void printOutputFiles(const std::vector<std::string>& files);
};

} // namespace CLI
} // namespace UnityContextGen