#include "AnalysisConfig.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

namespace UnityContextGen {
namespace Config {

ConfigurationManager& ConfigurationManager::getInstance() {
    static ConfigurationManager instance;
    return instance;
}

bool ConfigurationManager::loadConfig(const std::string& config_file_path) {
    std::string config_path = config_file_path;
    if (config_path.empty()) {
        config_path = getDefaultConfigPath();
    }
    
    m_config_file_path = config_path;
    
    if (!std::filesystem::exists(config_path)) {
        // Create default config
        setDefaultValues();
        loadFromEnvironmentVariables();
        return saveConfig(config_path);
    }
    
    try {
        std::ifstream config_file(config_path);
        if (!config_file.is_open()) {
            std::cerr << "Failed to open config file: " << config_path << std::endl;
            return false;
        }
        
        nlohmann::json config_json;
        config_file >> config_json;
        config_file.close();
        
        jsonToOptions(config_json);
        loadFromEnvironmentVariables(); // Environment variables override config file
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << std::endl;
        setDefaultValues();
        return false;
    }
}

bool ConfigurationManager::saveConfig(const std::string& config_file_path) const {
    std::string config_path = config_file_path;
    if (config_path.empty()) {
        config_path = m_config_file_path;
    }
    if (config_path.empty()) {
        config_path = getDefaultConfigPath();
    }
    
    try {
        // Ensure directory exists
        std::filesystem::path path(config_path);
        std::filesystem::create_directories(path.parent_path());
        
        std::ofstream config_file(config_path);
        if (!config_file.is_open()) {
            std::cerr << "Failed to create config file: " << config_path << std::endl;
            return false;
        }
        
        nlohmann::json config_json = optionsToJson();
        config_file << config_json.dump(2) << std::endl;
        config_file.close();
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save config: " << e.what() << std::endl;
        return false;
    }
}

nlohmann::json ConfigurationManager::optionsToJson() const {
    nlohmann::json j;
    
    // Core analysis options
    j["analysis"]["analyze_dependencies"] = m_analysis_options.analyze_dependencies;
    j["analysis"]["analyze_lifecycle"] = m_analysis_options.analyze_lifecycle;
    j["analysis"]["detect_patterns"] = m_analysis_options.detect_patterns;
    j["analysis"]["generate_metadata"] = m_analysis_options.generate_metadata;
    j["analysis"]["generate_context"] = m_analysis_options.generate_context;
    j["analysis"]["export_json"] = m_analysis_options.export_json;
    
    // Advanced analysis
    j["analysis"]["analyze_scriptable_objects"] = m_analysis_options.analyze_scriptable_objects;
    j["analysis"]["analyze_asset_references"] = m_analysis_options.analyze_asset_references;
    j["analysis"]["analyze_addressables"] = m_analysis_options.analyze_addressables;
    j["analysis"]["analyze_dots_ecs"] = m_analysis_options.analyze_dots_ecs;
    
    // Output options
    j["output"]["directory"] = m_analysis_options.output_directory;
    j["output"]["format"] = m_analysis_options.output_format;
    j["output"]["verbose"] = m_analysis_options.verbose_output;
    j["output"]["overwrite_existing"] = m_analysis_options.overwrite_existing;
    
    // Performance options
    j["performance"]["enable_multithreading"] = m_analysis_options.enable_multithreading;
    j["performance"]["enable_caching"] = m_analysis_options.enable_caching;
    j["performance"]["max_worker_threads"] = m_analysis_options.max_worker_threads;
    
    // AI Configuration
    const auto& ai = m_analysis_options.ai_config;
    j["ai"]["provider"] = ai.api_provider;
    j["ai"]["api_key"] = ai.api_key.empty() ? "***REDACTED***" : ai.api_key; // Don't save actual key
    j["ai"]["model_name"] = ai.model_name;
    j["ai"]["api_base_url"] = ai.api_base_url;
    j["ai"]["enable_ai_analysis"] = ai.enable_ai_analysis;
    j["ai"]["enable_code_suggestions"] = ai.enable_code_suggestions;
    j["ai"]["enable_architecture_analysis"] = ai.enable_architecture_analysis;
    j["ai"]["enable_performance_analysis"] = ai.enable_performance_analysis;
    j["ai"]["enable_security_analysis"] = ai.enable_security_analysis;
    j["ai"]["max_context_length"] = ai.max_context_length;
    j["ai"]["max_retries"] = ai.max_retries;
    j["ai"]["timeout_seconds"] = ai.timeout_seconds;
    
    // Real-time Configuration
    const auto& rt = m_analysis_options.realtime_config;
    j["realtime"]["enable_realtime_analysis"] = rt.enable_realtime_analysis;
    j["realtime"]["watch_file_changes"] = rt.watch_file_changes;
    j["realtime"]["auto_analyze_on_save"] = rt.auto_analyze_on_save;
    j["realtime"]["incremental_analysis"] = rt.incremental_analysis;
    j["realtime"]["file_change_debounce_ms"] = rt.file_change_debounce_ms;
    j["realtime"]["max_files_per_batch"] = rt.max_files_per_batch;
    j["realtime"]["analyze_dependencies_on_change"] = rt.analyze_dependencies_on_change;
    j["realtime"]["watch_cs_files"] = rt.watch_cs_files;
    j["realtime"]["watch_json_files"] = rt.watch_json_files;
    j["realtime"]["watch_asset_files"] = rt.watch_asset_files;
    
    return j;
}

void ConfigurationManager::jsonToOptions(const nlohmann::json& j) {
    // Core analysis options
    if (j.contains("analysis")) {
        const auto& analysis = j["analysis"];
        m_analysis_options.analyze_dependencies = analysis.value("analyze_dependencies", true);
        m_analysis_options.analyze_lifecycle = analysis.value("analyze_lifecycle", true);
        m_analysis_options.detect_patterns = analysis.value("detect_patterns", true);
        m_analysis_options.generate_metadata = analysis.value("generate_metadata", true);
        m_analysis_options.generate_context = analysis.value("generate_context", true);
        m_analysis_options.export_json = analysis.value("export_json", true);
        
        m_analysis_options.analyze_scriptable_objects = analysis.value("analyze_scriptable_objects", true);
        m_analysis_options.analyze_asset_references = analysis.value("analyze_asset_references", true);
        m_analysis_options.analyze_addressables = analysis.value("analyze_addressables", false);
        m_analysis_options.analyze_dots_ecs = analysis.value("analyze_dots_ecs", false);
    }
    
    // Output options
    if (j.contains("output")) {
        const auto& output = j["output"];
        m_analysis_options.output_directory = output.value("directory", "./unity_context_analysis");
        m_analysis_options.output_format = output.value("format", "all");
        m_analysis_options.verbose_output = output.value("verbose", false);
        m_analysis_options.overwrite_existing = output.value("overwrite_existing", false);
    }
    
    // Performance options
    if (j.contains("performance")) {
        const auto& perf = j["performance"];
        m_analysis_options.enable_multithreading = perf.value("enable_multithreading", true);
        m_analysis_options.enable_caching = perf.value("enable_caching", true);
        m_analysis_options.max_worker_threads = perf.value("max_worker_threads", 4);
    }
    
    // AI Configuration
    if (j.contains("ai")) {
        const auto& ai = j["ai"];
        m_analysis_options.ai_config.api_provider = ai.value("provider", "anthropic");
        m_analysis_options.ai_config.model_name = ai.value("model_name", "claude-3-5-sonnet-20241022");
        m_analysis_options.ai_config.api_base_url = ai.value("api_base_url", "https://api.anthropic.com");
        m_analysis_options.ai_config.enable_ai_analysis = ai.value("enable_ai_analysis", false);
        m_analysis_options.ai_config.enable_code_suggestions = ai.value("enable_code_suggestions", true);
        m_analysis_options.ai_config.enable_architecture_analysis = ai.value("enable_architecture_analysis", true);
        m_analysis_options.ai_config.enable_performance_analysis = ai.value("enable_performance_analysis", true);
        m_analysis_options.ai_config.enable_security_analysis = ai.value("enable_security_analysis", false);
        m_analysis_options.ai_config.max_context_length = ai.value("max_context_length", 200000);
        m_analysis_options.ai_config.max_retries = ai.value("max_retries", 3);
        m_analysis_options.ai_config.timeout_seconds = ai.value("timeout_seconds", 30);
        
        // Don't load API key from file for security
    }
    
    // Real-time Configuration
    if (j.contains("realtime")) {
        const auto& rt = j["realtime"];
        m_analysis_options.realtime_config.enable_realtime_analysis = rt.value("enable_realtime_analysis", false);
        m_analysis_options.realtime_config.watch_file_changes = rt.value("watch_file_changes", true);
        m_analysis_options.realtime_config.auto_analyze_on_save = rt.value("auto_analyze_on_save", true);
        m_analysis_options.realtime_config.incremental_analysis = rt.value("incremental_analysis", true);
        m_analysis_options.realtime_config.file_change_debounce_ms = rt.value("file_change_debounce_ms", 500);
        m_analysis_options.realtime_config.max_files_per_batch = rt.value("max_files_per_batch", 10);
        m_analysis_options.realtime_config.analyze_dependencies_on_change = rt.value("analyze_dependencies_on_change", true);
        m_analysis_options.realtime_config.watch_cs_files = rt.value("watch_cs_files", true);
        m_analysis_options.realtime_config.watch_json_files = rt.value("watch_json_files", false);
        m_analysis_options.realtime_config.watch_asset_files = rt.value("watch_asset_files", false);
    }
}

void ConfigurationManager::loadFromEnvironmentVariables() {
    // AI Configuration from environment variables
    std::string api_key = getEnvVar("ANTHROPIC_API_KEY");
    if (!api_key.empty()) {
        m_analysis_options.ai_config.api_key = api_key;
        m_analysis_options.ai_config.enable_ai_analysis = true;
    }
    
    // OpenAI fallback
    if (api_key.empty()) {
        api_key = getEnvVar("OPENAI_API_KEY");
        if (!api_key.empty()) {
            m_analysis_options.ai_config.api_key = api_key;
            m_analysis_options.ai_config.api_provider = "openai";
            m_analysis_options.ai_config.model_name = "gpt-4";
            m_analysis_options.ai_config.api_base_url = "https://api.openai.com";
            m_analysis_options.ai_config.enable_ai_analysis = true;
        }
    }
    
    // Other environment overrides
    if (getBoolEnvVar("UNITY_CONTEXT_VERBOSE", false)) {
        m_analysis_options.verbose_output = true;
    }
    
    if (getBoolEnvVar("UNITY_CONTEXT_REALTIME", false)) {
        m_analysis_options.realtime_config.enable_realtime_analysis = true;
    }
    
    std::string output_dir = getEnvVar("UNITY_CONTEXT_OUTPUT");
    if (!output_dir.empty()) {
        m_analysis_options.output_directory = output_dir;
    }
}

std::string ConfigurationManager::getEnvVar(const std::string& name, const std::string& default_value) const {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : default_value;
}

bool ConfigurationManager::getBoolEnvVar(const std::string& name, bool default_value) const {
    std::string value = getEnvVar(name, "");
    if (value.empty()) return default_value;
    
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return value == "true" || value == "1" || value == "yes" || value == "on";
}

int ConfigurationManager::getIntEnvVar(const std::string& name, int default_value) const {
    std::string value = getEnvVar(name, "");
    if (value.empty()) return default_value;
    
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

std::string ConfigurationManager::getDefaultConfigPath() const {
#ifdef _WIN32
    char* appdata = nullptr;
    size_t len = 0;
    _dupenv_s(&appdata, &len, "APPDATA");
    if (appdata) {
        std::string path = std::string(appdata) + "\\UnityContextGenerator\\config.json";
        free(appdata);
        return path;
    }
    return ".\\unity_context_config.json";
#else
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.unity_context_generator/config.json";
    }
    return "./unity_context_config.json";
#endif
}

std::string ConfigurationManager::getUnityConfigPath() const {
    return "./Library/UnityContextGenerator/config.json";
}

std::string ConfigurationManager::getGlobalConfigPath() const {
    return getDefaultConfigPath();
}

void ConfigurationManager::setDefaultValues() {
    m_analysis_options = AnalysisOptions(); // Use default constructor values
}

bool ConfigurationManager::validateConfiguration() const {
    return true; // Basic validation - can be expanded
}

std::vector<std::string> ConfigurationManager::getConfigurationWarnings() const {
    std::vector<std::string> warnings;
    
    if (m_analysis_options.ai_config.enable_ai_analysis && !hasValidAIConfig()) {
        warnings.push_back("AI analysis enabled but no valid API key configured");
    }
    
    if (m_analysis_options.realtime_config.enable_realtime_analysis) {
        warnings.push_back("Real-time analysis is experimental and may impact performance");
    }
    
    return warnings;
}

bool ConfigurationManager::hasValidAIConfig() const {
    return m_analysis_options.ai_config.is_valid();
}

void ConfigurationManager::setAnthropicApiKey(const std::string& api_key) {
    m_analysis_options.ai_config.api_key = api_key;
    m_analysis_options.ai_config.api_provider = "anthropic";
    m_analysis_options.ai_config.model_name = "claude-3-5-sonnet-20241022";
    m_analysis_options.ai_config.api_base_url = "https://api.anthropic.com";
    m_analysis_options.ai_config.enable_ai_analysis = !api_key.empty();
}

void ConfigurationManager::setOpenAIApiKey(const std::string& api_key) {
    m_analysis_options.ai_config.api_key = api_key;
    m_analysis_options.ai_config.api_provider = "openai";
    m_analysis_options.ai_config.model_name = "gpt-4";
    m_analysis_options.ai_config.api_base_url = "https://api.openai.com";
    m_analysis_options.ai_config.enable_ai_analysis = !api_key.empty();
}

void ConfigurationManager::enableRealTimeAnalysis(bool enable) {
    m_analysis_options.realtime_config.enable_realtime_analysis = enable;
}

void ConfigurationManager::setFileWatchDebounce(int milliseconds) {
    m_analysis_options.realtime_config.file_change_debounce_ms = milliseconds;
}

} // namespace Config
} // namespace UnityContextGen