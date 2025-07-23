#include "ClaudeAnalyzer.h"
#include "UnityAPIDetector.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

namespace UnityContextGen {
namespace AI {

using json = nlohmann::json;

ClaudeAnalyzer::ClaudeAnalyzer() : m_initialized(false) {
    m_http_client = std::make_unique<HTTPClient>();
}

bool ClaudeAnalyzer::initialize(const Config::AIAnalysisConfig& config) {
    m_config = config;
    
    if (!m_config.is_valid()) {
        logError("Invalid AI configuration - missing API key or model name");
        return false;
    }
    
    // Set HTTP client configuration
    m_http_client->setTimeout(m_config.timeout_seconds);
    m_http_client->setUserAgent("UnityContextGenerator/1.0 (Claude Integration)");
    
    m_initialized = true;
    logProgress("Claude API initialized successfully", 1.0f);
    
    return true;
}

AIAnalysisResult ClaudeAnalyzer::analyzeCode(const AIAnalysisRequest& request) {
    if (!m_initialized) {
        return createErrorResult("ClaudeAnalyzer not initialized");
    }
    
    logProgress("Starting AI code analysis...", 0.0f);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Detect Unity APIs in the code
    UnityAPIDetector detector;
    detector.setVerbose(false);
    auto api_usage = detector.analyzeCode(request.code_content, request.file_path);
    
    logProgress("Detected Unity APIs, analyzing with Claude...", 0.3f);
    
    // Create analysis context
    std::string analysis_context = createAnalysisContext(request.project_context, api_usage.detected_apis);
    
    // Construct prompt for Claude
    std::string prompt = constructPrompt(request, api_usage);
    std::string system_message = constructSystemMessage(request.analysis_type);
    
    logProgress("Sending request to Claude API...", 0.5f);
    
    // Make API request to Claude
    std::string claude_response = makeAPIRequest(prompt, system_message);
    
    if (claude_response.empty()) {
        return createErrorResult("Failed to get response from Claude API");
    }
    
    logProgress("Processing Claude response...", 0.8f);
    
    // Parse and structure the response
    AIAnalysisResult result = parseAPIResponse(claude_response, request);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    result.analysis_duration_ms = static_cast<int>(duration.count());
    
    logProgress("AI analysis completed", 1.0f);
    
    return result;
}

std::string ClaudeAnalyzer::constructPrompt(const AIAnalysisRequest& request, const UnityAPIUsage& api_usage) {
    std::stringstream prompt;
    
    prompt << "# Unity Code Analysis Request\n\n";
    
    // Project context
    if (!request.project_context.empty()) {
        prompt << "## Project Context\n";
        prompt << request.project_context << "\n\n";
    }
    
    // Component information
    prompt << "## Component: " << request.component_name << "\n";
    prompt << "**File:** " << request.file_path << "\n";
    prompt << "**Analysis Type:** " << request.analysis_type << "\n\n";
    
    // Detected Unity APIs
    if (!api_usage.detected_apis.empty()) {
        prompt << "## Detected Unity APIs\n";
        
        // Categorize APIs for better analysis
        if (!api_usage.input_apis.empty()) {
            prompt << "**Input APIs:** " << joinVector(api_usage.input_apis) << "\n";
        }
        if (!api_usage.physics_apis.empty()) {
            prompt << "**Physics APIs:** " << joinVector(api_usage.physics_apis) << "\n";
        }
        if (!api_usage.transform_apis.empty()) {
            prompt << "**Transform APIs:** " << joinVector(api_usage.transform_apis) << "\n";
        }
        if (!api_usage.component_apis.empty()) {
            prompt << "**Component APIs:** " << joinVector(api_usage.component_apis) << "\n";
        }
        if (!api_usage.performance_sensitive_apis.empty()) {
            prompt << "**⚠️ Performance-Sensitive APIs:** " << joinVector(api_usage.performance_sensitive_apis) << "\n";
        }
        prompt << "\n";
        
        // Detailed API usage with context
        prompt << "### API Usage Details\n";
        for (const auto& api : api_usage.detected_apis) {
            prompt << "- `" << api.full_call << "` (line " << api.line_number << ") in " << api.context << "\n";
        }
        prompt << "\n";
    }
    
    // Unity API database insights
    prompt << "## Unity API Insights\n";
    auto& db = UnityAPIDatabase::getInstance();
    for (const auto& api : api_usage.detected_apis) {
        auto issues = db.getCommonIssues(api.api_name);
        auto best_practices = db.getBestPractices(api.api_name);
        
        if (!issues.empty() || !best_practices.empty()) {
            prompt << "**" << api.api_name << ":**\n";
            if (!issues.empty()) {
                prompt << "  - Common Issues: " << joinVector(issues) << "\n";
            }
            if (!best_practices.empty()) {
                prompt << "  - Best Practices: " << joinVector(best_practices) << "\n";
            }
        }
    }
    prompt << "\n";
    
    // The actual code
    prompt << "## Code to Analyze\n";
    prompt << "```csharp\n";
    prompt << request.code_content << "\n";
    prompt << "```\n\n";
    
    // Analysis request
    prompt << "## Analysis Request\n";
    prompt << "Please provide a comprehensive analysis focusing on:\n";
    
    if (request.analysis_type == "code_review" || request.analysis_type.empty()) {
        prompt << "1. **Code Quality**: Overall code structure and organization\n";
        prompt << "2. **Unity Best Practices**: Proper use of Unity APIs and patterns\n";
        prompt << "3. **Performance Issues**: Potential performance bottlenecks\n";
        prompt << "4. **Architecture Suggestions**: Improvements to code design\n";
        prompt << "5. **Bug Prevention**: Potential issues or edge cases\n";
    }
    
    if (request.analysis_type == "performance") {
        prompt << "1. **Performance Bottlenecks**: Expensive operations and their impact\n";
        prompt << "2. **Memory Usage**: GC allocations and memory leaks\n";
        prompt << "3. **CPU Optimization**: Algorithmic improvements\n";
        prompt << "4. **Unity-Specific Optimizations**: Frame rate and rendering optimizations\n";
    }
    
    if (request.analysis_type == "architecture") {
        prompt << "1. **Design Patterns**: Current patterns and suggested improvements\n";
        prompt << "2. **Separation of Concerns**: How well responsibilities are divided\n";
        prompt << "3. **Maintainability**: Ease of modification and extension\n";
        prompt << "4. **Testability**: How well the code supports unit testing\n";
    }
    
    prompt << "\nProvide specific, actionable recommendations with code examples where helpful.\n";
    
    return prompt.str();
}

std::string ClaudeAnalyzer::constructSystemMessage(const std::string& analysis_type) {
    std::stringstream system_msg;
    
    system_msg << "You are an expert Unity developer and code reviewer with deep knowledge of Unity APIs, ";
    system_msg << "C# best practices, and game development patterns. ";
    system_msg << "You specialize in providing detailed, actionable code analysis and optimization suggestions.\n\n";
    
    system_msg << "When analyzing Unity code:\n";
    system_msg << "1. Focus on Unity-specific best practices and common pitfalls\n";
    system_msg << "2. Consider performance implications, especially for Update() methods\n";
    system_msg << "3. Identify potential null reference exceptions and other runtime errors\n";
    system_msg << "4. Suggest more efficient Unity API usage patterns\n";
    system_msg << "5. Recommend proper lifecycle method usage (Awake vs Start vs Update)\n";
    system_msg << "6. Consider mobile performance and cross-platform compatibility\n\n";
    
    system_msg << "Structure your response with clear sections:\n";
    system_msg << "- **Overall Assessment**: Brief summary of code quality\n";
    system_msg << "- **Key Issues**: Most important problems to address\n";
    system_msg << "- **Performance Concerns**: Performance-related recommendations\n";
    system_msg << "- **Best Practices**: Unity-specific improvements\n";
    system_msg << "- **Suggested Improvements**: Concrete code changes\n\n";
    
    system_msg << "Be specific and provide code examples for your suggestions when possible.";
    
    return system_msg.str();
}

std::string ClaudeAnalyzer::makeAPIRequest(const std::string& prompt, const std::string& system_message) {
    try {
        // Construct Claude API request JSON
        json request_json;
        request_json["model"] = m_config.model_name;
        request_json["max_tokens"] = 4000;
        request_json["temperature"] = 0.1; // Low temperature for consistent analysis
        
        request_json["system"] = system_message;
        
        json user_message;
        user_message["role"] = "user";
        user_message["content"] = prompt;
        request_json["messages"] = json::array({user_message});
        
        // Prepare headers
        std::vector<std::pair<std::string, std::string>> headers = {
            {"Content-Type", "application/json"},
            {"x-api-key", m_config.api_key},
            {"anthropic-version", "2023-06-01"}
        };
        
        // Make the request
        std::string request_body = request_json.dump();
        logError("Request URL: " + m_config.api_base_url + "/v1/messages");
        logError("Request body: " + request_body.substr(0, 1000) + "...");
        auto response = m_http_client->post(m_config.api_base_url + "/v1/messages", request_body, headers);
        
        if (!response.success) {
            logError("Claude API request failed: " + response.error_message);
            logError("Response body: " + response.body);
            logError("Status code: " + std::to_string(response.status_code));
            return "";
        }
        
        // Parse response JSON
        json response_json = json::parse(response.body);
        
        if (response_json.contains("error")) {
            logError("Claude API error: " + response_json["error"]["message"].get<std::string>());
            return "";
        }
        
        if (response_json.contains("content") && response_json["content"].is_array() && 
            !response_json["content"].empty()) {
            return response_json["content"][0]["text"].get<std::string>();
        }
        
        logError("Unexpected Claude API response format");
        return "";
        
    } catch (const std::exception& e) {
        logError("Error making Claude API request: " + std::string(e.what()));
        return "";
    }
}

AIAnalysisResult ClaudeAnalyzer::parseAPIResponse(const std::string& response, const AIAnalysisRequest& request) {
    AIAnalysisResult result;
    result.success = true;
    result.overall_assessment = response;
    
    // Extract structured information from Claude's response
    // This is a simplified parser - could be more sophisticated
    
    std::vector<std::string> lines;
    std::istringstream iss(response);
    std::string line;
    
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    // Parse sections
    std::string current_section;
    for (const auto& line : lines) {
        if (line.find("**") != std::string::npos) {
            // This is a section header
            if (line.find("Key Issues") != std::string::npos || line.find("Issues") != std::string::npos) {
                current_section = "issues";
            } else if (line.find("Performance") != std::string::npos) {
                current_section = "performance";
            } else if (line.find("Best Practices") != std::string::npos || line.find("Improvements") != std::string::npos) {
                current_section = "best_practices";
            } else if (line.find("Suggestions") != std::string::npos) {
                current_section = "suggestions";
            }
        } else if (line.find("- ") == 0 || line.find("• ") == 0) {
            // This is a bullet point
            std::string item = line.substr(2); // Remove bullet point
            
            if (current_section == "issues") {
                result.potential_issues.push_back(item);
            } else if (current_section == "performance") {
                result.performance_optimizations.push_back(item);
            } else if (current_section == "best_practices") {
                result.best_practices.push_back(item);
            } else if (current_section == "suggestions") {
                result.suggestions.push_back(item);
            }
        }
    }
    
    // Extract Unity-specific insights
    if (response.find("GetComponent") != std::string::npos && response.find("cache") != std::string::npos) {
        result.unity_best_practices.push_back("Cache GetComponent results to avoid repeated calls");
    }
    
    if (response.find("Update") != std::string::npos && response.find("performance") != std::string::npos) {
        result.lifecycle_recommendations.push_back("Optimize Update() method for better performance");
    }
    
    // Set confidence score based on response quality
    result.confidence_score = 0.8f; // Default confidence
    if (result.potential_issues.size() > 2 && result.suggestions.size() > 2) {
        result.confidence_score = 0.9f; // High confidence for comprehensive analysis
    }
    
    return result;
}

std::string ClaudeAnalyzer::createAnalysisContext(const std::string& project_metadata,
                                                 const std::vector<DetectedAPI>& unity_apis) {
    std::stringstream context;
    
    context << "Project Type: Unity Game/Application\n";
    context << "Detected APIs: ";
    
    std::unordered_set<std::string> unique_apis;
    for (const auto& api : unity_apis) {
        unique_apis.insert(api.api_name);
    }
    
    bool first = true;
    for (const auto& api : unique_apis) {
        if (!first) context << ", ";
        context << api;
        first = false;
    }
    context << "\n";
    
    if (!project_metadata.empty()) {
        context << "\nProject Context:\n" << project_metadata << "\n";
    }
    
    return context.str();
}

AIAnalysisResult ClaudeAnalyzer::createErrorResult(const std::string& error_message) {
    AIAnalysisResult result;
    result.success = false;
    result.error_message = error_message;
    result.confidence_score = 0.0f;
    result.analysis_duration_ms = 0;
    return result;
}

void ClaudeAnalyzer::logError(const std::string& message) {
    std::cerr << "[ClaudeAnalyzer ERROR] " << message << std::endl;
}

void ClaudeAnalyzer::logProgress(const std::string& message, float progress) {
    if (m_progress_callback) {
        m_progress_callback(message, progress);
    }
    
    // Also log to console if verbose
    std::cout << "[ClaudeAnalyzer] " << message << " (" << (progress * 100) << "%)" << std::endl;
}

std::string ClaudeAnalyzer::joinVector(const std::vector<std::string>& vec) {
    if (vec.empty()) return "";
    
    std::stringstream ss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << vec[i];
    }
    return ss.str();
}

// Quick factory method for easy use
std::unique_ptr<ClaudeAnalyzer> createClaudeAnalyzer(const std::string& api_key) {
    auto analyzer = std::make_unique<ClaudeAnalyzer>();
    
    Config::AIAnalysisConfig config;
    config.api_key = api_key;
    config.enable_ai_analysis = true;
    
    if (analyzer->initialize(config)) {
        return analyzer;
    }
    
    return nullptr;
}

} // namespace AI
} // namespace UnityContextGen