#include "UnityPatternDetector.h"
#include <algorithm>
#include <regex>

namespace UnityContextGen {
namespace Unity {

UnityPatternDetector::UnityPatternDetector() {
}

void UnityPatternDetector::analyzeProject(const std::vector<MonoBehaviourInfo>& monobehaviours,
                                         const ComponentGraph& dependency_graph) {
    m_monobehaviours = monobehaviours;
    m_dependency_graph = dependency_graph;
    m_detected_patterns.clear();
    
    detectSingletonPattern();
    detectObjectPoolingPattern();
    detectStatePattern();
    detectObserverPattern();
    detectComponentCompositionPattern();
    detectServiceLocatorPattern();
    detectFactoryPattern();
    detectCommandPattern();
    detectMVCPattern();
    detectECSPattern();
}

void UnityPatternDetector::detectSingletonPattern() {
    for (const auto& mb_info : m_monobehaviours) {
        if (hasSingletonCharacteristics(mb_info)) {
            PatternInstance pattern;
            pattern.pattern_type = UnityPattern::SingletonMonoBehaviour;
            pattern.pattern_name = "Singleton MonoBehaviour";
            pattern.involved_components.push_back(mb_info.class_name);
            pattern.description = "MonoBehaviour implementing singleton pattern for global access";
            pattern.purpose = "Ensure single instance and provide global access point";
            
            pattern.evidence.push_back("Static instance field");
            pattern.evidence.push_back("Instance access method");
            pattern.evidence.push_back("DontDestroyOnLoad usage");
            
            pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
            m_detected_patterns.push_back(pattern);
        }
    }
}

void UnityPatternDetector::detectObjectPoolingPattern() {
    for (const auto& mb_info : m_monobehaviours) {
        if (hasPoolingCharacteristics(mb_info)) {
            PatternInstance pattern;
            pattern.pattern_type = UnityPattern::ObjectPooling;
            pattern.pattern_name = "Object Pooling";
            pattern.involved_components.push_back(mb_info.class_name);
            pattern.description = "Reuses objects to avoid frequent allocation/deallocation";
            pattern.purpose = "Optimize performance by reusing game objects";
            
            pattern.evidence.push_back("Pool collection field");
            pattern.evidence.push_back("Get/Return methods");
            pattern.evidence.push_back("SetActive usage");
            
            pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
            m_detected_patterns.push_back(pattern);
        }
    }
}

void UnityPatternDetector::detectStatePattern() {
    if (hasStatePatternCharacteristics(m_monobehaviours)) {
        std::vector<std::string> state_components = findComponentsByPattern(".*State.*");
        
        if (!state_components.empty()) {
            PatternInstance pattern;
            pattern.pattern_type = UnityPattern::StatePattern;
            pattern.pattern_name = "State Pattern";
            pattern.involved_components = state_components;
            pattern.description = "Implements state-based behavior with state transitions";
            pattern.purpose = "Manage complex object behavior through states";
            
            pattern.evidence.push_back("Multiple state classes");
            pattern.evidence.push_back("State transition methods");
            pattern.evidence.push_back("Current state field");
            
            pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
            m_detected_patterns.push_back(pattern);
        }
    }
}

void UnityPatternDetector::detectObserverPattern() {
    if (hasObserverCharacteristics(m_monobehaviours)) {
        std::vector<std::string> observer_components;
        
        for (const auto& mb_info : m_monobehaviours) {
            bool has_events = false;
            bool has_listeners = false;
            
            for (const auto& method : mb_info.unity_methods) {
                if (method.find("Event") != std::string::npos || 
                    method.find("Notify") != std::string::npos) {
                    has_events = true;
                }
                if (method.find("Subscribe") != std::string::npos || 
                    method.find("Listen") != std::string::npos) {
                    has_listeners = true;
                }
            }
            
            if (has_events || has_listeners) {
                observer_components.push_back(mb_info.class_name);
            }
        }
        
        if (!observer_components.empty()) {
            PatternInstance pattern;
            pattern.pattern_type = UnityPattern::ObserverPattern;
            pattern.pattern_name = "Observer Pattern";
            pattern.involved_components = observer_components;
            pattern.description = "Implements event-driven communication between objects";
            pattern.purpose = "Decouple objects through event notifications";
            
            pattern.evidence.push_back("Event declarations");
            pattern.evidence.push_back("Subscribe/Unsubscribe methods");
            pattern.evidence.push_back("Notification methods");
            
            pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
            m_detected_patterns.push_back(pattern);
        }
    }
}

void UnityPatternDetector::detectComponentCompositionPattern() {
    for (const auto& mb_info : m_monobehaviours) {
        if (mb_info.component_dependencies.size() >= 3) {
            PatternInstance pattern;
            pattern.pattern_type = UnityPattern::ComponentComposition;
            pattern.pattern_name = "Component Composition";
            pattern.involved_components.push_back(mb_info.class_name);
            pattern.description = "Combines multiple components to create complex behavior";
            pattern.purpose = "Build complex functionality through component composition";
            
            pattern.evidence.push_back("Multiple component dependencies");
            pattern.evidence.push_back("GetComponent calls");
            pattern.evidence.push_back("RequireComponent attributes");
            
            pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
            m_detected_patterns.push_back(pattern);
        }
    }
}

void UnityPatternDetector::detectServiceLocatorPattern() {
    std::vector<std::string> service_components = findComponentsByPattern(".*Service.*|.*Manager.*");
    
    if (service_components.size() >= 2) {
        PatternInstance pattern;
        pattern.pattern_type = UnityPattern::ServiceLocator;
        pattern.pattern_name = "Service Locator";
        pattern.involved_components = service_components;
        pattern.description = "Provides centralized access to services";
        pattern.purpose = "Manage and provide access to game services";
        
        pattern.evidence.push_back("Service/Manager classes");
        pattern.evidence.push_back("Service registration");
        pattern.evidence.push_back("Service lookup methods");
        
        pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
        m_detected_patterns.push_back(pattern);
    }
}

void UnityPatternDetector::detectFactoryPattern() {
    std::vector<std::string> factory_components = findComponentsByPattern(".*Factory.*|.*Creator.*|.*Builder.*");
    
    if (!factory_components.empty()) {
        PatternInstance pattern;
        pattern.pattern_type = UnityPattern::FactoryPattern;
        pattern.pattern_name = "Factory Pattern";
        pattern.involved_components = factory_components;
        pattern.description = "Creates objects without specifying exact classes";
        pattern.purpose = "Encapsulate object creation logic";
        
        pattern.evidence.push_back("Factory/Creator classes");
        pattern.evidence.push_back("Create methods");
        pattern.evidence.push_back("Instantiate calls");
        
        pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
        m_detected_patterns.push_back(pattern);
    }
}

void UnityPatternDetector::detectCommandPattern() {
    std::vector<std::string> command_components = findComponentsByPattern(".*Command.*|.*Action.*");
    
    if (!command_components.empty()) {
        PatternInstance pattern;
        pattern.pattern_type = UnityPattern::CommandPattern;
        pattern.pattern_name = "Command Pattern";
        pattern.involved_components = command_components;
        pattern.description = "Encapsulates requests as objects";
        pattern.purpose = "Support undo/redo operations and request queuing";
        
        pattern.evidence.push_back("Command/Action classes");
        pattern.evidence.push_back("Execute methods");
        pattern.evidence.push_back("Undo/Redo support");
        
        pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
        m_detected_patterns.push_back(pattern);
    }
}

void UnityPatternDetector::detectMVCPattern() {
    std::vector<std::string> mvc_components;
    
    for (const auto& component : findComponentsByPattern(".*Controller.*")) {
        mvc_components.push_back(component);
    }
    for (const auto& component : findComponentsByPattern(".*View.*")) {
        mvc_components.push_back(component);
    }
    for (const auto& component : findComponentsByPattern(".*Model.*")) {
        mvc_components.push_back(component);
    }
    
    if (mvc_components.size() >= 3) {
        PatternInstance pattern;
        pattern.pattern_type = UnityPattern::MVC_Pattern;
        pattern.pattern_name = "MVC Pattern";
        pattern.involved_components = mvc_components;
        pattern.description = "Separates application logic into Model, View, and Controller";
        pattern.purpose = "Improve code organization and maintainability";
        
        pattern.evidence.push_back("Controller classes");
        pattern.evidence.push_back("View classes");
        pattern.evidence.push_back("Model classes");
        
        pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
        m_detected_patterns.push_back(pattern);
    }
}

void UnityPatternDetector::detectECSPattern() {
    std::vector<std::string> ecs_components;
    
    for (const auto& component : findComponentsByPattern(".*Entity.*|.*Component.*|.*System.*")) {
        ecs_components.push_back(component);
    }
    
    if (ecs_components.size() >= 3) {
        PatternInstance pattern;
        pattern.pattern_type = UnityPattern::ECS_Pattern;
        pattern.pattern_name = "Entity Component System";
        pattern.involved_components = ecs_components;
        pattern.description = "Implements data-oriented design with entities, components, and systems";
        pattern.purpose = "Optimize performance and improve code modularity";
        
        pattern.evidence.push_back("Entity classes");
        pattern.evidence.push_back("Component data");
        pattern.evidence.push_back("System logic");
        
        pattern.confidence_score = calculateConfidenceScore(pattern.evidence);
        m_detected_patterns.push_back(pattern);
    }
}

bool UnityPatternDetector::hasSingletonCharacteristics(const MonoBehaviourInfo& mb_info) const {
    bool has_static_instance = false;
    bool has_instance_access = false;
    
    for (const auto& field : mb_info.serialized_fields) {
        if (field.find("static") != std::string::npos && 
            field.find("instance") != std::string::npos) {
            has_static_instance = true;
        }
    }
    
    for (const auto& method : mb_info.custom_methods) {
        if (method == "Instance" || method == "GetInstance") {
            has_instance_access = true;
        }
    }
    
    return has_static_instance || has_instance_access;
}

bool UnityPatternDetector::hasPoolingCharacteristics(const MonoBehaviourInfo& mb_info) const {
    bool has_pool_collection = false;
    bool has_pool_methods = false;
    
    for (const auto& field : mb_info.serialized_fields) {
        if (field.find("Pool") != std::string::npos || 
            field.find("Queue") != std::string::npos ||
            field.find("List") != std::string::npos) {
            has_pool_collection = true;
        }
    }
    
    for (const auto& method : mb_info.custom_methods) {
        if (method.find("Get") != std::string::npos || 
            method.find("Return") != std::string::npos ||
            method.find("Pool") != std::string::npos) {
            has_pool_methods = true;
        }
    }
    
    return has_pool_collection && has_pool_methods;
}

bool UnityPatternDetector::hasStatePatternCharacteristics(const std::vector<MonoBehaviourInfo>& components) const {
    int state_classes = 0;
    
    for (const auto& mb_info : components) {
        if (mb_info.class_name.find("State") != std::string::npos) {
            state_classes++;
        }
    }
    
    return state_classes >= 2;
}

bool UnityPatternDetector::hasObserverCharacteristics(const std::vector<MonoBehaviourInfo>& components) const {
    bool has_events = false;
    bool has_listeners = false;
    
    for (const auto& mb_info : components) {
        for (const auto& method : mb_info.custom_methods) {
            if (method.find("Event") != std::string::npos || 
                method.find("Notify") != std::string::npos) {
                has_events = true;
            }
            if (method.find("Subscribe") != std::string::npos || 
                method.find("Listen") != std::string::npos) {
                has_listeners = true;
            }
        }
    }
    
    return has_events && has_listeners;
}

std::vector<std::string> UnityPatternDetector::findComponentsByPattern(const std::string& pattern_regex) const {
    std::vector<std::string> matching_components;
    std::regex pattern(pattern_regex);
    
    for (const auto& mb_info : m_monobehaviours) {
        if (std::regex_match(mb_info.class_name, pattern)) {
            matching_components.push_back(mb_info.class_name);
        }
    }
    
    return matching_components;
}

float UnityPatternDetector::calculateConfidenceScore(const std::vector<std::string>& evidence) const {
    float base_score = 0.5f;
    float evidence_bonus = evidence.size() * 0.15f;
    return std::min(0.95f, base_score + evidence_bonus);
}

std::vector<PatternInstance> UnityPatternDetector::getDetectedPatterns() const {
    return m_detected_patterns;
}

std::vector<PatternInstance> UnityPatternDetector::getPatternsByType(UnityPattern pattern_type) const {
    std::vector<PatternInstance> patterns;
    
    for (const auto& pattern : m_detected_patterns) {
        if (pattern.pattern_type == pattern_type) {
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::string UnityPatternDetector::generatePatternSummary() const {
    std::string summary = "Unity Design Patterns Detected:\n\n";
    
    auto frequency = getPatternFrequency();
    for (const auto& pair : frequency) {
        summary += getPatternName(pair.first) + ": " + std::to_string(pair.second) + " instances\n";
    }
    
    summary += "\nDetailed Analysis:\n";
    for (const auto& pattern : m_detected_patterns) {
        summary += "\n" + pattern.pattern_name + " (Confidence: " + 
                   std::to_string(static_cast<int>(pattern.confidence_score * 100)) + "%)\n";
        summary += "  Components: ";
        for (size_t i = 0; i < pattern.involved_components.size(); ++i) {
            if (i > 0) summary += ", ";
            summary += pattern.involved_components[i];
        }
        summary += "\n  Purpose: " + pattern.purpose + "\n";
    }
    
    return summary;
}

std::map<UnityPattern, int> UnityPatternDetector::getPatternFrequency() const {
    std::map<UnityPattern, int> frequency;
    
    for (const auto& pattern : m_detected_patterns) {
        frequency[pattern.pattern_type]++;
    }
    
    return frequency;
}

std::string UnityPatternDetector::getPatternName(UnityPattern pattern) const {
    switch (pattern) {
        case UnityPattern::SingletonMonoBehaviour: return "Singleton MonoBehaviour";
        case UnityPattern::ObjectPooling: return "Object Pooling";
        case UnityPattern::StatePattern: return "State Pattern";
        case UnityPattern::ObserverPattern: return "Observer Pattern";
        case UnityPattern::ComponentComposition: return "Component Composition";
        case UnityPattern::ServiceLocator: return "Service Locator";
        case UnityPattern::FactoryPattern: return "Factory Pattern";
        case UnityPattern::CommandPattern: return "Command Pattern";
        case UnityPattern::MVC_Pattern: return "MVC Pattern";
        case UnityPattern::ECS_Pattern: return "Entity Component System";
        default: return "Unknown Pattern";
    }
}

std::string UnityPatternDetector::getPatternDescription(UnityPattern pattern) const {
    switch (pattern) {
        case UnityPattern::SingletonMonoBehaviour: 
            return "MonoBehaviour implementing singleton pattern for global access";
        case UnityPattern::ObjectPooling: 
            return "Reuses objects to avoid frequent allocation/deallocation";
        case UnityPattern::StatePattern: 
            return "Implements state-based behavior with state transitions";
        case UnityPattern::ObserverPattern: 
            return "Implements event-driven communication between objects";
        case UnityPattern::ComponentComposition: 
            return "Combines multiple components to create complex behavior";
        case UnityPattern::ServiceLocator: 
            return "Provides centralized access to services";
        case UnityPattern::FactoryPattern: 
            return "Creates objects without specifying exact classes";
        case UnityPattern::CommandPattern: 
            return "Encapsulates requests as objects";
        case UnityPattern::MVC_Pattern: 
            return "Separates application logic into Model, View, and Controller";
        case UnityPattern::ECS_Pattern: 
            return "Implements data-oriented design with entities, components, and systems";
        default: 
            return "Unknown pattern";
    }
}

} // namespace Unity
} // namespace UnityContextGen