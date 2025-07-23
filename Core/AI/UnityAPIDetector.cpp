#include "UnityAPIDetector.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace UnityContextGen {
namespace AI {

UnityAPIDetector::UnityAPIDetector() : m_verbose(false) {
    initializeAPIDatabase();
    initializeRegexPatterns();
}

UnityAPIUsage UnityAPIDetector::analyzeFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        logVerbose("Failed to open file: " + file_path);
        return UnityAPIUsage{};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code_content = buffer.str();
    file.close();

    return analyzeCode(code_content, file_path);
}

UnityAPIUsage UnityAPIDetector::analyzeCode(const std::string& code_content, const std::string& file_path) {
    UnityAPIUsage usage;
    usage.file_path = file_path;
    
    // Extract class name from code
    std::regex class_regex(R"(class\s+(\w+)(?:\s*:\s*[\w,\s]+)?)");
    std::smatch class_match;
    if (std::regex_search(code_content, class_match, class_regex)) {
        usage.class_name = class_match[1].str();
    }
    
    // Detect Unity API calls
    usage.detected_apis = detectUnityAPICalls(code_content);
    
    // Categorize APIs
    categorizeAPIs(usage);
    
    logVerbose("Detected " + std::to_string(usage.detected_apis.size()) + " Unity API calls in " + file_path);
    
    return usage;
}

std::vector<UnityAPIUsage> UnityAPIDetector::analyzeProject(const std::vector<std::string>& file_paths) {
    std::vector<UnityAPIUsage> results;
    
    for (const auto& file_path : file_paths) {
        if (file_path.find(".cs") != std::string::npos) { // Only analyze C# files
            auto usage = analyzeFile(file_path);
            if (!usage.detected_apis.empty()) {
                results.push_back(usage);
            }
        }
    }
    
    return results;
}

std::vector<DetectedAPI> UnityAPIDetector::detectUnityAPICalls(const std::string& code_content) {
    std::vector<DetectedAPI> detected_apis;
    
    // Common Unity API patterns
    std::vector<std::pair<std::regex, std::string>> api_patterns = {
        // Component access
        {std::regex(R"(GetComponent<(\w+)>\(\))"), "GetComponent"},
        {std::regex(R"(GetComponents?<(\w+)>\(\))"), "GetComponent"},
        {std::regex(R"(GetComponentInChildren<(\w+)>\(\))"), "GetComponentInChildren"},
        {std::regex(R"(GetComponentInParent<(\w+)>\(\))"), "GetComponentInParent"},
        {std::regex(R"(AddComponent<(\w+)>\(\))"), "AddComponent"},
        
        // Input system
        {std::regex(R"(Input\.GetAxis\s*\(\s*\"([^\"]+)\"\s*\))"), "Input.GetAxis"},
        {std::regex(R"(Input\.GetButton\w*\s*\(\s*\"([^\"]+)\"\s*\))"), "Input.GetButton"},
        {std::regex(R"(Input\.GetKey\w*\s*\(\s*[\w\.\s\"]+\s*\))"), "Input.GetKey"},
        {std::regex(R"(Input\.GetMouseButton\w*\s*\(\s*\d+\s*\))"), "Input.GetMouseButton"},
        
        // Time and deltaTime
        {std::regex(R"(Time\.deltaTime\b)"), "Time.deltaTime"},
        {std::regex(R"(Time\.time\b)"), "Time.time"},
        {std::regex(R"(Time\.fixedDeltaTime\b)"), "Time.fixedDeltaTime"},
        {std::regex(R"(Time\.timeScale\b)"), "Time.timeScale"},
        
        // Transform operations
        {std::regex(R"(transform\.position\b)"), "transform.position"},
        {std::regex(R"(transform\.rotation\b)"), "transform.rotation"},
        {std::regex(R"(transform\.localPosition\b)"), "transform.localPosition"},
        {std::regex(R"(transform\.localRotation\b)"), "transform.localRotation"},
        {std::regex(R"(transform\.localScale\b)"), "transform.localScale"},
        {std::regex(R"(transform\.Translate\s*\()"), "transform.Translate"},
        {std::regex(R"(transform\.Rotate\s*\()"), "transform.Rotate"},
        {std::regex(R"(transform\.LookAt\s*\()"), "transform.LookAt"},
        
        // Physics
        {std::regex(R"(Rigidbody\.velocity\b)"), "Rigidbody.velocity"},
        {std::regex(R"(Rigidbody\.AddForce\s*\()"), "Rigidbody.AddForce"},
        {std::regex(R"(Physics\.Raycast\s*\()"), "Physics.Raycast"},
        {std::regex(R"(Collider\.bounds\b)"), "Collider.bounds"},
        
        // GameObject operations
        {std::regex(R"(GameObject\.Find\s*\(\s*\"([^\"]+)\"\s*\))"), "GameObject.Find"},
        {std::regex(R"(GameObject\.FindWithTag\s*\(\s*\"([^\"]+)\"\s*\))"), "GameObject.FindWithTag"},
        {std::regex(R"(GameObject\.CreatePrimitive\s*\()"), "GameObject.CreatePrimitive"},
        {std::regex(R"(Instantiate\s*\()"), "Instantiate"},
        {std::regex(R"(Destroy\s*\()"), "Destroy"},
        
        // UI
        {std::regex(R"(Button\.onClick\b)"), "Button.onClick"},
        {std::regex(R"(Text\.text\b)"), "Text.text"},
        {std::regex(R"(Image\.sprite\b)"), "Image.sprite"},
        
        // Audio
        {std::regex(R"(AudioSource\.Play\s*\()"), "AudioSource.Play"},
        {std::regex(R"(AudioSource\.clip\b)"), "AudioSource.clip"},
        
        // Coroutines
        {std::regex(R"(StartCoroutine\s*\()"), "StartCoroutine"},
        {std::regex(R"(StopCoroutine\s*\()"), "StopCoroutine"},
        
        // Debug
        {std::regex(R"(Debug\.Log\w*\s*\()"), "Debug.Log"},
        {std::regex(R"(Debug\.DrawRay\s*\()"), "Debug.DrawRay"},
    };
    
    auto lines = splitIntoLines(code_content);
    
    for (const auto& [pattern, api_name] : api_patterns) {
        std::sregex_iterator iter(code_content.begin(), code_content.end(), pattern);
        std::sregex_iterator end;
        
        while (iter != end) {
            DetectedAPI detected;
            detected.api_name = api_name;
            detected.full_call = iter->str();
            
            // Find line number
            size_t position = iter->position();
            detected.line_number = findLineNumber(code_content, position);
            
            // Find containing method
            detected.context = findContainingMethod(code_content, detected.line_number);
            detected.usage_pattern = detected.context;
            
            detected_apis.push_back(detected);
            ++iter;
        }
    }
    
    // Sort by line number
    std::sort(detected_apis.begin(), detected_apis.end(), 
              [](const DetectedAPI& a, const DetectedAPI& b) {
                  return a.line_number < b.line_number;
              });
    
    return detected_apis;
}

void UnityAPIDetector::categorizeAPIs(UnityAPIUsage& usage) {
    for (const auto& api : usage.detected_apis) {
        std::string category = getAPICategory(api.api_name);
        
        if (category == "Input") {
            usage.input_apis.push_back(api.api_name);
        } else if (category == "Physics") {
            usage.physics_apis.push_back(api.api_name);
        } else if (category == "Transform") {
            usage.transform_apis.push_back(api.api_name);
        } else if (category == "Component") {
            usage.component_apis.push_back(api.api_name);
        } else if (category == "Lifecycle") {
            usage.lifecycle_apis.push_back(api.api_name);
        } else if (category == "UI") {
            usage.ui_apis.push_back(api.api_name);
        } else if (category == "Audio") {
            usage.audio_apis.push_back(api.api_name);
        } else if (category == "Rendering") {
            usage.rendering_apis.push_back(api.api_name);
        }
        
        if (isPerformanceSensitive(api.api_name)) {
            usage.performance_sensitive_apis.push_back(api.api_name);
        }
    }
    
    // Remove duplicates
    auto removeDuplicates = [](std::vector<std::string>& vec) {
        std::sort(vec.begin(), vec.end());
        vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    };
    
    removeDuplicates(usage.input_apis);
    removeDuplicates(usage.physics_apis);
    removeDuplicates(usage.transform_apis);
    removeDuplicates(usage.component_apis);
    removeDuplicates(usage.lifecycle_apis);
    removeDuplicates(usage.ui_apis);
    removeDuplicates(usage.audio_apis);
    removeDuplicates(usage.rendering_apis);
    removeDuplicates(usage.performance_sensitive_apis);
}

std::string UnityAPIDetector::getAPICategory(const std::string& api_name) {
    auto& db = UnityAPIDatabase::getInstance();
    return db.getAPICategory(api_name);
}

bool UnityAPIDetector::isPerformanceSensitive(const std::string& api_name) {
    auto& db = UnityAPIDatabase::getInstance();
    return db.isPerformanceSensitive(api_name);
}

std::vector<std::string> UnityAPIDetector::splitIntoLines(const std::string& content) {
    std::vector<std::string> lines;
    std::istringstream stream(content);
    std::string line;
    
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

int UnityAPIDetector::findLineNumber(const std::string& content, size_t position) {
    return static_cast<int>(std::count(content.begin(), content.begin() + position, '\n')) + 1;
}

std::string UnityAPIDetector::findContainingMethod(const std::string& code_content, int line_number) {
    auto lines = splitIntoLines(code_content);
    
    // Search backwards from the line to find method declaration
    for (int i = line_number - 1; i >= 0; --i) {
        if (i < lines.size()) {
            std::regex method_regex(R"((public|private|protected)\s+[\w<>\[\]]+\s+(\w+)\s*\([^)]*\))");
            std::smatch match;
            if (std::regex_search(lines[i], match, method_regex)) {
                return match[2].str(); // Return method name
            }
        }
    }
    
    return "Unknown";
}

void UnityAPIDetector::initializeAPIDatabase() {
    // This will be populated by UnityAPIDatabase
    auto& db = UnityAPIDatabase::getInstance();
    // Database is initialized in its constructor
}

void UnityAPIDetector::initializeRegexPatterns() {
    // Regex patterns are initialized in detectUnityAPICalls
}

std::unordered_map<std::string, int> UnityAPIDetector::getAPIFrequency(const std::vector<UnityAPIUsage>& usages) {
    std::unordered_map<std::string, int> frequency;
    
    for (const auto& usage : usages) {
        for (const auto& api : usage.detected_apis) {
            frequency[api.api_name]++;
        }
    }
    
    return frequency;
}

std::vector<std::string> UnityAPIDetector::getMostUsedAPIs(const std::vector<UnityAPIUsage>& usages, int top_count) {
    auto frequency = getAPIFrequency(usages);
    
    std::vector<std::pair<std::string, int>> pairs(frequency.begin(), frequency.end());
    std::sort(pairs.begin(), pairs.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> result;
    for (int i = 0; i < std::min(top_count, static_cast<int>(pairs.size())); ++i) {
        result.push_back(pairs[i].first);
    }
    
    return result;
}

void UnityAPIDetector::logVerbose(const std::string& message) {
    if (m_verbose) {
        std::cout << "[UnityAPIDetector] " << message << std::endl;
    }
}

// UnityAPIDatabase Implementation
UnityAPIDatabase& UnityAPIDatabase::getInstance() {
    static UnityAPIDatabase instance;
    return instance;
}

UnityAPIDatabase::UnityAPIDatabase() {
    loadDefaultAPIs();
}

void UnityAPIDatabase::loadDefaultAPIs() {
    // Component APIs
    addAPI("GetComponent", "Component", "Gets a component attached to the GameObject", true,
           {"Expensive call", "Should be cached", "Can return null"},
           {"Cache result in Awake/Start", "Use TryGetComponent for null safety"},
           "Cache in Awake(), don't call repeatedly");
    
    addAPI("GetComponentInChildren", "Component", "Gets a component in children GameObjects", true,
           {"Very expensive", "Recursive search", "Can return null"},
           {"Cache result", "Consider component references instead"},
           "Use sparingly, cache results");
    
    addAPI("AddComponent", "Component", "Adds a component to the GameObject", true,
           {"Can only add one of same type", "Expensive operation"},
           {"Check if component exists first", "Use generic version"},
           "Avoid in runtime loops");
    
    // Input APIs
    addAPI("Input.GetAxis", "Input", "Gets input axis value (-1 to 1)", false,
           {"Legacy input system", "String-based lookup"},
           {"Use new Input System", "Cache axis names"},
           "Fine to call in Update()");
    
    addAPI("Input.GetButton", "Input", "Gets button press state", false,
           {"Legacy input system", "String-based lookup"},
           {"Use new Input System", "Cache button names"},
           "Fine to call in Update()");
    
    // Time APIs
    addAPI("Time.deltaTime", "Time", "Time since last frame", false,
           {},
           {"Use for frame-independent movement"},
           "Essential for smooth animations");
    
    addAPI("Time.time", "Time", "Time since game started", false,
           {},
           {"Use for timing calculations"},
           "Good for timers and delays");
    
    // Transform APIs
    addAPI("transform.position", "Transform", "World position of the GameObject", false,
           {"Setting bypasses physics", "Expensive world-to-local conversion"},
           {"Use Rigidbody for physics objects", "Consider localPosition for UI"},
           "Avoid setting every frame for physics objects");
    
    addAPI("transform.Translate", "Transform", "Moves the transform relative to reference frame", false,
           {"Can cause tunneling", "Bypasses collision"},
           {"Use for non-physics movement", "Consider physics for realistic movement"},
           "Good for UI and non-physics objects");
    
    // Physics APIs
    addAPI("Rigidbody.velocity", "Physics", "Velocity vector of the rigidbody", false,
           {},
           {"Preferred over transform manipulation", "Respects physics"},
           "Best for physics-based movement");
    
    addAPI("Rigidbody.AddForce", "Physics", "Adds force to the rigidbody", false,
           {},
           {"More realistic than velocity", "Consider force mode"},
           "Use for realistic physics interactions");
    
    addAPI("Physics.Raycast", "Physics", "Casts a ray and detects collisions", true,
           {"Expensive operation", "Can impact performance"},
           {"Limit raycasts per frame", "Use layers for filtering"},
           "Cache results when possible");
    
    // GameObject APIs
    addAPI("GameObject.Find", "GameObject", "Finds GameObject by name", true,
           {"Very expensive", "String-based search", "Can return null"},
           {"Cache references", "Use direct references instead"},
           "Avoid entirely - use references");
    
    addAPI("Instantiate", "GameObject", "Creates copy of an object", true,
           {"Expensive operation", "Can cause GC pressure"},
           {"Use object pooling", "Instantiate in batches"},
           "Consider object pooling for frequent use");
    
    addAPI("Destroy", "GameObject", "Destroys GameObject", true,
           {"Not immediate", "Can leave null references"},
           {"Check for null after Destroy", "Use DestroyImmediate carefully"},
           "Be careful with references after destroy");
    
    // UI APIs
    addAPI("Button.onClick", "UI", "Button click event", false,
           {},
           {"Subscribe in Start, unsubscribe in OnDestroy"},
           "Handle event subscription properly");
    
    // Audio APIs
    addAPI("AudioSource.Play", "Audio", "Plays the audio clip", false,
           {"Requires AudioSource component", "Can interrupt previous audio"},
           {"Check if AudioSource exists", "Consider PlayOneShot"},
           "Good for simple audio playback");
    
    // Debug APIs
    addAPI("Debug.Log", "Debug", "Logs message to console", true,
           {"Can impact performance in builds", "Should be removed/disabled"},
           {"Use conditional compilation", "Remove from release builds"},
           "Remove or disable in production");
}

void UnityAPIDatabase::addAPI(const std::string& name, const std::string& category, 
                             const std::string& description, bool is_perf_sensitive,
                             const std::vector<std::string>& issues,
                             const std::vector<std::string>& best_practices,
                             const std::string& recommended_usage) {
    APIInfo info;
    info.name = name;
    info.category = category;
    info.description = description;
    info.is_performance_sensitive = is_perf_sensitive;
    info.common_issues = issues;
    info.best_practices = best_practices;
    info.recommended_usage = recommended_usage;
    
    m_api_database[name] = info;
}

const UnityAPIDatabase::APIInfo* UnityAPIDatabase::getAPIInfo(const std::string& api_name) {
    auto it = m_api_database.find(api_name);
    return (it != m_api_database.end()) ? &it->second : nullptr;
}

std::string UnityAPIDatabase::getAPICategory(const std::string& api_name) {
    auto info = getAPIInfo(api_name);
    return info ? info->category : "Unknown";
}

bool UnityAPIDatabase::isPerformanceSensitive(const std::string& api_name) {
    auto info = getAPIInfo(api_name);
    return info ? info->is_performance_sensitive : false;
}

bool UnityAPIDatabase::isKnownAPI(const std::string& api_name) {
    return m_api_database.find(api_name) != m_api_database.end();
}

std::vector<std::string> UnityAPIDatabase::getCommonIssues(const std::string& api_name) {
    auto info = getAPIInfo(api_name);
    return info ? info->common_issues : std::vector<std::string>{};
}

std::vector<std::string> UnityAPIDatabase::getBestPractices(const std::string& api_name) {
    auto info = getAPIInfo(api_name);
    return info ? info->best_practices : std::vector<std::string>{};
}

} // namespace AI
} // namespace UnityContextGen