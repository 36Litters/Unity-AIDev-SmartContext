#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>

namespace UnityContextGen {
namespace Config {

struct AIAnalysisConfig {
    std::string api_provider = "anthropic"; // "anthropic", "openai", "none"
    std::string api_key = "";
    std::string model_name = "claude-3-5-sonnet-20241022";
    std::string api_base_url = "https://api.anthropic.com";
    
    bool enable_ai_analysis = false;
    bool enable_code_suggestions = true;
    bool enable_architecture_analysis = true;
    bool enable_performance_analysis = true;
    bool enable_security_analysis = false;
    
    int max_context_length = 200000; // Claude 3.5 Sonnet context limit
    int max_retries = 3;
    int timeout_seconds = 30;
    
    bool is_valid() const {
        return !api_key.empty() && !model_name.empty() && !api_base_url.empty();
    }
};

struct RealTimeConfig {
    bool enable_realtime_analysis = false;
    bool watch_file_changes = true;
    bool auto_analyze_on_save = true;
    bool incremental_analysis = true;
    
    int file_change_debounce_ms = 500; // Wait 500ms after last change
    int max_files_per_batch = 10;
    bool analyze_dependencies_on_change = true;
    
    // File type filters
    bool watch_cs_files = true;
    bool watch_json_files = false;
    bool watch_asset_files = false;
};

struct AnalysisOptions {
    // Core analysis options
    bool analyze_dependencies = true;
    bool analyze_lifecycle = true;
    bool detect_patterns = true;
    bool generate_metadata = true;
    bool generate_context = true;
    bool export_json = true;
    
    // Advanced analysis options
    bool analyze_scriptable_objects = true;
    bool analyze_asset_references = true;
    bool analyze_addressables = false;
    bool analyze_dots_ecs = false; // Future feature
    
    // Output options
    std::string output_directory = "./unity_context_analysis";
    std::string output_format = "all"; // "json", "markdown", "llm", "all"
    bool verbose_output = false;
    bool overwrite_existing = false;
    
    // Performance options
    bool enable_multithreading = true;
    bool enable_caching = true;
    int max_worker_threads = 4;
    
    // AI Integration
    AIAnalysisConfig ai_config;
    
    // Real-time options
    RealTimeConfig realtime_config;
};

class ConfigurationManager {
public:
    static ConfigurationManager& getInstance();
    
    // Configuration loading/saving
    bool loadConfig(const std::string& config_file_path = "");
    bool saveConfig(const std::string& config_file_path = "") const;
    
    // Configuration access
    AnalysisOptions& getAnalysisOptions() { return m_analysis_options; }
    const AnalysisOptions& getAnalysisOptions() const { return m_analysis_options; }
    
    AIAnalysisConfig& getAIConfig() { return m_analysis_options.ai_config; }
    const AIAnalysisConfig& getAIConfig() const { return m_analysis_options.ai_config; }
    
    RealTimeConfig& getRealTimeConfig() { return m_analysis_options.realtime_config; }
    const RealTimeConfig& getRealTimeConfig() const { return m_analysis_options.realtime_config; }
    
    // Configuration validation
    bool validateConfiguration() const;
    std::vector<std::string> getConfigurationWarnings() const;
    
    // Environment variable support
    void loadFromEnvironmentVariables();
    
    // Unity-specific configuration paths
    std::string getUnityConfigPath() const;
    std::string getGlobalConfigPath() const;
    
    // AI API configuration
    bool hasValidAIConfig() const;
    void setAnthropicApiKey(const std::string& api_key);
    void setOpenAIApiKey(const std::string& api_key);
    
    // Real-time configuration
    void enableRealTimeAnalysis(bool enable);
    void setFileWatchDebounce(int milliseconds);

private:
    ConfigurationManager() = default;
    ~ConfigurationManager() = default;
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;
    
    AnalysisOptions m_analysis_options;
    std::string m_config_file_path;
    
    // Helper methods
    nlohmann::json optionsToJson() const;
    void jsonToOptions(const nlohmann::json& json);
    std::string getDefaultConfigPath() const;
    void setDefaultValues();
    
    // Environment variable helpers
    std::string getEnvVar(const std::string& name, const std::string& default_value = "") const;
    bool getBoolEnvVar(const std::string& name, bool default_value = false) const;
    int getIntEnvVar(const std::string& name, int default_value = 0) const;
};

// Convenience macros
#define ANALYSIS_CONFIG() UnityContextGen::Config::ConfigurationManager::getInstance().getAnalysisOptions()
#define AI_CONFIG() UnityContextGen::Config::ConfigurationManager::getInstance().getAIConfig()
#define REALTIME_CONFIG() UnityContextGen::Config::ConfigurationManager::getInstance().getRealTimeConfig()

} // namespace Config
} // namespace UnityContextGen