#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <regex>

namespace UnityContextGen {
namespace AI {

struct DetectedAPI {
    std::string api_name;
    std::string full_call;
    std::string context; // Method where it's used
    int line_number;
    std::string usage_pattern; // "Update", "Awake", "OnCollision", etc.
};

struct UnityAPIUsage {
    std::string file_path;
    std::string class_name;
    std::vector<DetectedAPI> detected_apis;
    
    // Categorized APIs
    std::vector<std::string> input_apis;
    std::vector<std::string> physics_apis;
    std::vector<std::string> transform_apis;
    std::vector<std::string> component_apis;
    std::vector<std::string> lifecycle_apis;
    std::vector<std::string> ui_apis;
    std::vector<std::string> audio_apis;
    std::vector<std::string> rendering_apis;
    std::vector<std::string> performance_sensitive_apis;
};

class UnityAPIDetector {
public:
    UnityAPIDetector();
    ~UnityAPIDetector() = default;
    
    // Main detection methods
    UnityAPIUsage analyzeFile(const std::string& file_path);
    UnityAPIUsage analyzeCode(const std::string& code_content, const std::string& file_path = "");
    
    // Batch analysis
    std::vector<UnityAPIUsage> analyzeProject(const std::vector<std::string>& file_paths);
    
    // API pattern detection
    std::vector<DetectedAPI> detectUnityAPICalls(const std::string& code_content);
    std::vector<std::string> extractMethodCalls(const std::string& code_content);
    std::vector<std::string> extractPropertyAccess(const std::string& code_content);
    std::vector<std::string> extractComponentReferences(const std::string& code_content);
    
    // Categorization
    void categorizeAPIs(UnityAPIUsage& usage);
    std::string getAPICategory(const std::string& api_name);
    bool isPerformanceSensitive(const std::string& api_name);
    
    // Analysis helpers
    std::vector<std::string> getUsagePatterns(const std::string& code_content, const std::string& api_name);
    std::string extractUsageContext(const std::string& code_content, const std::string& api_call, int line_number);
    
    // Statistics
    std::unordered_map<std::string, int> getAPIFrequency(const std::vector<UnityAPIUsage>& usages);
    std::vector<std::string> getMostUsedAPIs(const std::vector<UnityAPIUsage>& usages, int top_count = 10);
    
    // Configuration
    void addCustomAPIPattern(const std::string& pattern, const std::string& category);
    void setVerbose(bool verbose) { m_verbose = verbose; }

private:
    bool m_verbose;
    
    // API pattern databases
    std::unordered_set<std::string> m_known_unity_apis;
    std::unordered_map<std::string, std::string> m_api_categories;
    std::unordered_set<std::string> m_performance_sensitive_apis;
    
    // Regex patterns for different API types
    std::vector<std::regex> m_method_patterns;
    std::vector<std::regex> m_property_patterns;
    std::vector<std::regex> m_component_patterns;
    std::vector<std::regex> m_generic_patterns;
    
    // Initialization
    void initializeAPIDatabase();
    void initializeRegexPatterns();
    
    // Detection helpers
    bool isUnityAPI(const std::string& api_call);
    std::string normalizeAPIName(const std::string& api_call);
    std::vector<std::string> splitIntoLines(const std::string& content);
    int findLineNumber(const std::string& content, size_t position);
    
    // Context analysis
    std::string findContainingMethod(const std::string& code_content, int line_number);
    bool isInUpdateLoop(const std::string& context);
    bool isInPerformanceCriticalSection(const std::string& context);
    
    // Logging
    void logVerbose(const std::string& message);
};

// Unity API database - known Unity APIs and their categories
class UnityAPIDatabase {
public:
    static UnityAPIDatabase& getInstance();
    
    // API information
    struct APIInfo {
        std::string name;
        std::string category;
        std::string description;
        bool is_performance_sensitive;
        std::vector<std::string> common_issues;
        std::vector<std::string> best_practices;
        std::string recommended_usage;
    };
    
    // Database access
    const APIInfo* getAPIInfo(const std::string& api_name);
    std::vector<std::string> getAPIsByCategory(const std::string& category);
    std::vector<std::string> getPerformanceSensitiveAPIs();
    
    // Query methods
    bool isKnownAPI(const std::string& api_name);
    std::string getAPICategory(const std::string& api_name);
    bool isPerformanceSensitive(const std::string& api_name);
    std::vector<std::string> getCommonIssues(const std::string& api_name);
    std::vector<std::string> getBestPractices(const std::string& api_name);

private:
    UnityAPIDatabase();
    std::unordered_map<std::string, APIInfo> m_api_database;
    
    void loadDefaultAPIs();
    void addAPI(const std::string& name, const std::string& category, 
                const std::string& description, bool is_perf_sensitive,
                const std::vector<std::string>& issues = {},
                const std::vector<std::string>& best_practices = {},
                const std::string& recommended_usage = "");
};

} // namespace AI
} // namespace UnityContextGen