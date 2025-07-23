#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <queue>

namespace UnityContextGen {
namespace RealTime {

enum class FileChangeType {
    Created,
    Modified,
    Deleted,
    Renamed
};

struct FileChangeEvent {
    std::string file_path;
    FileChangeType change_type;
    std::chrono::steady_clock::time_point timestamp;
    std::string old_path; // For rename events
    
    bool operator<(const FileChangeEvent& other) const {
        return timestamp > other.timestamp; // For priority queue (oldest first)
    }
};

struct AnalysisTrigger {
    std::vector<std::string> changed_files;
    std::chrono::steady_clock::time_point trigger_time;
    bool is_batch;
    std::string trigger_reason;
};

class FileWatcher {
public:
    using ChangeCallback = std::function<void(const FileChangeEvent&)>;
    using BatchCallback = std::function<void(const std::vector<FileChangeEvent>&)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    FileWatcher();
    ~FileWatcher();
    
    // Configuration
    void setWatchDirectory(const std::string& directory);
    void setFileFilter(const std::vector<std::string>& extensions);
    void setDebounceTime(std::chrono::milliseconds debounce_ms);
    void setMaxBatchSize(int max_batch_size);
    
    // Callbacks
    void setChangeCallback(ChangeCallback callback) { m_change_callback = callback; }
    void setBatchCallback(BatchCallback callback) { m_batch_callback = callback; }
    void setErrorCallback(ErrorCallback callback) { m_error_callback = callback; }
    
    // Control
    bool startWatching();
    void stopWatching();
    bool isWatching() const { return m_is_watching.load(); }
    
    // Manual triggers
    void forceAnalysisTrigger(const std::string& reason = "manual");
    void scanDirectory(); // One-time scan for changes
    
    // Status
    std::vector<std::string> getWatchedDirectories() const;
    std::vector<std::string> getWatchedExtensions() const;
    int getPendingEventsCount() const;
    
private:
    // Configuration
    std::string m_watch_directory;
    std::vector<std::string> m_file_extensions;
    std::chrono::milliseconds m_debounce_time{500};
    int m_max_batch_size{10};
    
    // State
    std::atomic<bool> m_is_watching{false};
    std::atomic<bool> m_should_stop{false};
    
    // Threading
    std::unique_ptr<std::thread> m_watch_thread;
    std::unique_ptr<std::thread> m_process_thread;
    mutable std::mutex m_events_mutex;
    mutable std::mutex m_config_mutex;
    
    // Event processing
    std::priority_queue<FileChangeEvent> m_pending_events;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> m_last_change_time;
    
    // Callbacks
    ChangeCallback m_change_callback;
    BatchCallback m_batch_callback;
    ErrorCallback m_error_callback;
    
    // Platform-specific implementation
    class WatcherImpl;
    std::unique_ptr<WatcherImpl> m_impl;
    
    // Thread functions
    void watchThreadFunction();
    void processThreadFunction();
    
    // Event handling
    void addEvent(const FileChangeEvent& event);
    std::vector<FileChangeEvent> getReadyEvents();
    bool shouldTriggerAnalysis(const std::string& file_path);
    bool isValidFileExtension(const std::string& file_path) const;
    
    // Utility
    void logError(const std::string& message);
    void logDebug(const std::string& message);
};

class IncrementalAnalyzer {
public:
    IncrementalAnalyzer();
    ~IncrementalAnalyzer() = default;
    
    // Configuration
    void setProjectDirectory(const std::string& project_dir);
    void enableIncrementalMode(bool enable) { m_incremental_enabled = enable; }
    
    // Analysis triggers
    using AnalysisCallback = std::function<void(const std::vector<std::string>&, const std::string&)>;
    void setAnalysisCallback(AnalysisCallback callback) { m_analysis_callback = callback; }
    
    // Main interface
    void processFileChanges(const std::vector<FileChangeEvent>& events);
    void scheduleFullAnalysis(const std::string& reason);
    
    // Cache management
    void invalidateCache(const std::string& file_path);
    void invalidateAllCaches();
    bool isCached(const std::string& file_path) const;
    
    // Dependency tracking
    void updateDependencyGraph(const std::string& file_path);
    std::vector<std::string> getAffectedFiles(const std::string& changed_file);
    
    // Statistics
    int getCacheHitCount() const { return m_cache_hits; }
    int getCacheMissCount() const { return m_cache_misses; }
    std::chrono::milliseconds getAverageAnalysisTime() const;

private:
    std::string m_project_directory;
    bool m_incremental_enabled{true};
    
    // Caching
    struct FileAnalysisCache {
        std::string file_path;
        std::chrono::file_time_type last_modified;
        std::string analysis_result_hash;
        std::vector<std::string> dependencies;
        std::chrono::steady_clock::time_point cache_time;
    };
    
    std::unordered_map<std::string, FileAnalysisCache> m_file_cache;
    mutable std::mutex m_cache_mutex;
    
    // Statistics
    std::atomic<int> m_cache_hits{0};
    std::atomic<int> m_cache_misses{0};
    std::vector<std::chrono::milliseconds> m_analysis_times;
    mutable std::mutex m_stats_mutex;
    
    // Callbacks
    AnalysisCallback m_analysis_callback;
    
    // Helper methods
    bool needsAnalysis(const std::string& file_path);
    void updateCache(const std::string& file_path, const std::string& result_hash);
    std::vector<std::string> extractDependencies(const std::string& file_path);
    std::string computeFileHash(const std::string& file_path);
};

// Real-time analysis coordinator
class RealTimeAnalysisCoordinator {
public:
    RealTimeAnalysisCoordinator();
    ~RealTimeAnalysisCoordinator();
    
    // Initialization
    bool initialize(const std::string& project_directory);
    void shutdown();
    
    // Configuration from config system
    void loadConfiguration();
    
    // Analysis integration
    using ProjectAnalysisCallback = std::function<void(const std::vector<std::string>&, bool)>;
    void setProjectAnalysisCallback(ProjectAnalysisCallback callback) { m_project_analysis_callback = callback; }
    
    // Control
    bool start();
    void stop();
    bool isRunning() const { return m_is_running.load(); }
    
    // Manual controls
    void triggerFullAnalysis();
    void triggerIncrementalAnalysis(const std::vector<std::string>& files);
    
    // Statistics and monitoring
    struct Statistics {
        int total_file_changes;
        int triggered_analyses;
        int cache_hits;
        int cache_misses;
        std::chrono::milliseconds average_analysis_time;
        std::chrono::steady_clock::time_point last_analysis;
    };
    
    Statistics getStatistics() const;
    void resetStatistics();

private:
    std::atomic<bool> m_is_running{false};
    std::string m_project_directory;
    
    // Components
    std::unique_ptr<FileWatcher> m_file_watcher;
    std::unique_ptr<IncrementalAnalyzer> m_incremental_analyzer;
    
    // Configuration
    Config::RealTimeConfig m_config;
    
    // Callbacks
    ProjectAnalysisCallback m_project_analysis_callback;
    
    // Statistics
    mutable Statistics m_statistics;
    mutable std::mutex m_stats_mutex;
    
    // Event handlers
    void onFileChanged(const FileChangeEvent& event);
    void onBatchFileChanged(const std::vector<FileChangeEvent>& events);
    void onFileWatchError(const std::string& error);
    
    // Analysis coordination
    void scheduleAnalysis(const std::vector<std::string>& files, bool is_incremental);
    
    // Utility
    void updateStatistics(const std::string& operation, std::chrono::milliseconds duration = {});
    void logEvent(const std::string& message);
};

} // namespace RealTime
} // namespace UnityContextGen