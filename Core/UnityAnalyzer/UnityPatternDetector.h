#pragma once

#include "MonoBehaviourAnalyzer.h"
#include "ComponentDependencyAnalyzer.h"
#include <string>
#include <vector>
#include <map>

namespace UnityContextGen {
namespace Unity {

enum class UnityPattern {
    SingletonMonoBehaviour,
    ObjectPooling,
    StatePattern,
    ObserverPattern,
    ComponentComposition,
    ServiceLocator,
    FactoryPattern,
    CommandPattern,
    MVC_Pattern,
    ECS_Pattern,
    Unknown
};

struct PatternInstance {
    UnityPattern pattern_type;
    std::string pattern_name;
    std::vector<std::string> involved_components;
    std::string description;
    std::string purpose;
    float confidence_score;
    std::vector<std::string> evidence;
};

class UnityPatternDetector {
public:
    UnityPatternDetector();
    ~UnityPatternDetector() = default;

    void analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours,
                       const ComponentGraph& dependency_graph);
    
    std::vector<PatternInstance> getDetectedPatterns() const;
    std::vector<PatternInstance> getPatternsByType(UnityPattern pattern_type) const;
    
    std::string generatePatternSummary() const;
    std::map<UnityPattern, int> getPatternFrequency() const;

private:
    std::vector<MonoBehaviourInfo> m_monobehaviours;
    ComponentGraph m_dependency_graph;
    std::vector<PatternInstance> m_detected_patterns;
    
    void detectSingletonPattern();
    void detectObjectPoolingPattern();
    void detectStatePattern();
    void detectObserverPattern();
    void detectComponentCompositionPattern();
    void detectServiceLocatorPattern();
    void detectFactoryPattern();
    void detectCommandPattern();
    void detectMVCPattern();
    void detectECSPattern();
    
    bool hasSingletonCharacteristics(const MonoBehaviourInfo& mb_info) const;
    bool hasPoolingCharacteristics(const MonoBehaviourInfo& mb_info) const;
    bool hasStatePatternCharacteristics(const std::vector<MonoBehaviourInfo>& components) const;
    bool hasObserverCharacteristics(const std::vector<MonoBehaviourInfo>& components) const;
    
    std::vector<std::string> findComponentsByPattern(const std::string& pattern_regex) const;
    float calculateConfidenceScore(const std::vector<std::string>& evidence) const;
    
    std::string getPatternName(UnityPattern pattern) const;
    std::string getPatternDescription(UnityPattern pattern) const;
};

} // namespace Unity
} // namespace UnityContextGen