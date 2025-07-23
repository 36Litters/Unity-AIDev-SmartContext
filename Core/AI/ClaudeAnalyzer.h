#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "Configuration/AnalysisConfig.h"

namespace UnityContextGen {
namespace AI {

struct AIAnalysisRequest {
    std::string project_context;
    std::string code_content;
    std::vector<std::string> unity_apis_used;
    std::string analysis_type; // "code_review", "architecture", "performance", "security"
    std::string component_name;
    std::string file_path;
};

struct AIAnalysisResult {
    bool success;
    std::string error_message;
    
    // Analysis results
    std::string overall_assessment;
    std::vector<std::string> suggestions;
    std::vector<std::string> potential_issues;
    std::vector<std::string> best_practices;
    std::vector<std::string> performance_optimizations;
    std::vector<std::string> security_concerns;
    
    // Architecture insights
    std::string architecture_pattern;
    std::vector<std::string> design_improvements;
    std::string maintainability_score;
    std::string complexity_assessment;
    
    // Unity-specific insights
    std::vector<std::string> unity_best_practices;
    std::vector<std::string> lifecycle_recommendations;
    std::vector<std::string> performance_tips;
    
    // Confidence metrics
    float confidence_score;
    int analysis_duration_ms;
};

// Forward declarations
class UnityAPIDatabase;
struct DetectedAPI;
struct UnityAPIUsage;

class ClaudeAnalyzer {
public:
    ClaudeAnalyzer();
    ~ClaudeAnalyzer() = default;
    
    // Configuration
    bool initialize(const Config::AIAnalysisConfig& config);
    bool isInitialized() const { return m_initialized; }
    
    // Main analysis methods
    AIAnalysisResult analyzeCode(const AIAnalysisRequest& request);
    AIAnalysisResult reviewArchitecture(const std::string& project_context, 
                                       const std::vector<std::string>& component_summaries);
    AIAnalysisResult suggestImprovements(const std::string& code_content, 
                                        const std::string& component_name);
    
    // Batch analysis
    std::vector<AIAnalysisResult> analyzeBatch(const std::vector<AIAnalysisRequest>& requests);
    
    // Context management
    std::string createAnalysisContext(const std::string& project_metadata,
                                     const std::vector<DetectedAPI>& unity_apis);
    
    // Progress callback
    using ProgressCallback = std::function<void(const std::string&, float)>;
    void setProgressCallback(ProgressCallback callback) { m_progress_callback = callback; }

private:
    bool m_initialized;
    Config::AIAnalysisConfig m_config;
    ProgressCallback m_progress_callback;
    
    // HTTP client for API calls
    std::unique_ptr<class HTTPClient> m_http_client;
    
    // API interaction
    std::string makeAPIRequest(const std::string& prompt, const std::string& system_message = "");
    std::string constructPrompt(const AIAnalysisRequest& request, const UnityAPIUsage& api_usage);
    std::string constructSystemMessage(const std::string& analysis_type);
    
    // Response parsing
    AIAnalysisResult parseAPIResponse(const std::string& response, const AIAnalysisRequest& request);
    std::vector<std::string> extractListItems(const std::string& text, const std::string& section_marker);
    float extractConfidenceScore(const std::string& response);
    
    // Context preparation
    std::string prepareCodeContext(const AIAnalysisRequest& request);
    std::string prepareUnityAPIContext(const std::vector<std::string>& apis);
    
    // Helper methods
    std::string joinVector(const std::vector<std::string>& vec);
    
    // Retry logic
    AIAnalysisResult retryableAPICall(const std::function<AIAnalysisResult()>& api_call);
    
    // Error handling
    AIAnalysisResult createErrorResult(const std::string& error_message);
    void logError(const std::string& message);
    void logProgress(const std::string& message, float progress);
};

// HTTP client interface for API calls
class HTTPClient {
public:
    struct HTTPResponse {
        int status_code;
        std::string body;
        std::string error_message;
        bool success;
    };
    
    HTTPClient();
    ~HTTPClient();
    
    HTTPResponse post(const std::string& url, 
                     const std::string& body,
                     const std::vector<std::pair<std::string, std::string>>& headers);
    
    void setTimeout(int seconds);
    void setUserAgent(const std::string& user_agent);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace AI
} // namespace UnityContextGen