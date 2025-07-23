#include "ArchitectureAnalyzer.h"
#include <algorithm>
#include <numeric>
#include <sstream>

namespace UnityContextGen {
namespace Context {

ArchitectureAnalyzer::ArchitectureAnalyzer() {
}

ArchitectureInsight ArchitectureAnalyzer::analyzeArchitecture(const Metadata::ProjectMetadata& project_metadata) const {
    ArchitectureInsight insight;
    
    insight.primary_type = detectPrimaryArchitecture(project_metadata);
    insight.secondary_types = detectSecondaryArchitectures(project_metadata);
    insight.confidence_score = calculateArchitectureConfidence(insight.primary_type, project_metadata);
    insight.evidence = gatherArchitectureEvidence(insight.primary_type, project_metadata);
    insight.description = getArchitectureDescription(insight.primary_type);
    
    insight.characteristics.push_back("Unity MonoBehaviour-based structure");
    
    if (insight.primary_type == ArchitectureType::ComponentBasedEntity) {
        insight.characteristics.push_back("Component composition pattern");
        insight.characteristics.push_back("GameObject-centric design");
    } else if (insight.primary_type == ArchitectureType::ModelViewController) {
        insight.characteristics.push_back("Clear separation of UI and logic");
        insight.characteristics.push_back("Data flow through model updates");
    } else if (insight.primary_type == ArchitectureType::EventDrivenArchitecture) {
        insight.characteristics.push_back("Observer pattern implementation");
        insight.characteristics.push_back("Loose coupling through events");
    }
    
    if (project_metadata.systems.core_systems.size() > 3) {
        insight.characteristics.push_back("Multi-system architecture");
    }
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        insight.characteristics.push_back("Contains circular dependencies");
    } else {
        insight.characteristics.push_back("Clean dependency hierarchy");
    }
    
    return insight;
}

std::vector<SystemCohesion> ArchitectureAnalyzer::analyzeSystemCohesion(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<SystemCohesion> cohesion_analysis;
    
    for (const auto& system_group : project_metadata.systems.system_groups) {
        SystemCohesion cohesion;
        cohesion.system_name = system_group.first;
        cohesion.components = system_group.second;
        
        cohesion.cohesion_score = calculateSystemCohesion(system_group.second, project_metadata);
        cohesion.coupling_score = calculateSystemCoupling(system_group.second, project_metadata);
        
        std::string system_name_lower = system_group.first;
        std::transform(system_name_lower.begin(), system_name_lower.end(), system_name_lower.begin(), ::tolower);
        
        if (system_name_lower.find("player") != std::string::npos) {
            cohesion.responsibility = "Manage player behavior, input, and state";
        } else if (system_name_lower.find("ai") != std::string::npos || system_name_lower.find("enemy") != std::string::npos) {
            cohesion.responsibility = "Control enemy behavior and artificial intelligence";
        } else if (system_name_lower.find("ui") != std::string::npos) {
            cohesion.responsibility = "Handle user interface and interaction";
        } else if (system_name_lower.find("physics") != std::string::npos) {
            cohesion.responsibility = "Manage physics simulation and collision";
        } else if (system_name_lower.find("audio") != std::string::npos) {
            cohesion.responsibility = "Control sound effects and music";
        } else if (system_name_lower.find("camera") != std::string::npos) {
            cohesion.responsibility = "Manage camera movement and behavior";
        } else {
            cohesion.responsibility = "Handle " + system_group.first + " functionality";
        }
        
        cohesion.is_well_designed = (cohesion.cohesion_score > 0.7f && cohesion.coupling_score < 0.5f);
        
        cohesion_analysis.push_back(cohesion);
    }
    
    std::sort(cohesion_analysis.begin(), cohesion_analysis.end(),
              [](const SystemCohesion& a, const SystemCohesion& b) {
                  return a.cohesion_score > b.cohesion_score;
              });
    
    return cohesion_analysis;
}

ArchitectureHealth ArchitectureAnalyzer::assessArchitectureHealth(const Metadata::ProjectMetadata& project_metadata) const {
    ArchitectureHealth health;
    
    health.separation_of_concerns_score = assessSeparationOfConcerns(project_metadata);
    health.dependency_inversion_score = assessDependencyInversion(project_metadata);
    health.single_responsibility_score = assessSingleResponsibility(project_metadata);
    health.open_closed_score = assessOpenClosed(project_metadata);
    
    health.overall_health_score = (health.separation_of_concerns_score +
                                  health.dependency_inversion_score +
                                  health.single_responsibility_score +
                                  health.open_closed_score) / 4.0f;
    
    health.violations = identifySOLIDViolations(project_metadata);
    health.strengths = identifyArchitecturalStrengths(project_metadata);
    
    return health;
}

ArchitectureType ArchitectureAnalyzer::detectPrimaryArchitecture(const Metadata::ProjectMetadata& project_metadata) const {
    std::map<ArchitectureType, float> architecture_scores;
    
    bool has_mvc_pattern = false;
    bool has_ecs_pattern = false;
    bool has_event_pattern = false;
    bool has_service_pattern = false;
    
    for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
        if (pattern_pair.first.find("MVC") != std::string::npos) {
            has_mvc_pattern = true;
        } else if (pattern_pair.first.find("ECS") != std::string::npos || 
                   pattern_pair.first.find("Entity") != std::string::npos) {
            has_ecs_pattern = true;
        } else if (pattern_pair.first.find("Observer") != std::string::npos) {
            has_event_pattern = true;
        } else if (pattern_pair.first.find("Service") != std::string::npos) {
            has_service_pattern = true;
        }
    }
    
    if (has_mvc_pattern) {
        architecture_scores[ArchitectureType::ModelViewController] = 0.8f;
    }
    
    if (has_ecs_pattern) {
        architecture_scores[ArchitectureType::EntityComponentSystem] = 0.9f;
    }
    
    if (has_event_pattern) {
        architecture_scores[ArchitectureType::EventDrivenArchitecture] = 0.7f;
    }
    
    if (has_service_pattern) {
        architecture_scores[ArchitectureType::ServiceOrientedArchitecture] = 0.6f;
    }
    
    int manager_count = 0;
    int controller_count = 0;
    
    for (const auto& comp_pair : project_metadata.components) {
        std::string class_name = comp_pair.first;
        std::transform(class_name.begin(), class_name.end(), class_name.begin(), ::tolower);
        
        if (class_name.find("manager") != std::string::npos) {
            manager_count++;
        }
        if (class_name.find("controller") != std::string::npos) {
            controller_count++;
        }
    }
    
    if (manager_count >= 3) {
        architecture_scores[ArchitectureType::ServiceOrientedArchitecture] += 0.3f;
    }
    
    if (controller_count >= 2) {
        architecture_scores[ArchitectureType::ModelViewController] += 0.2f;
    }
    
    if (project_metadata.systems.system_groups.size() >= 4) {
        architecture_scores[ArchitectureType::LayeredArchitecture] = 0.5f;
    }
    
    architecture_scores[ArchitectureType::ComponentBasedEntity] = 0.4f;
    
    ArchitectureType primary_type = ArchitectureType::ComponentBasedEntity;
    float max_score = 0.0f;
    
    for (const auto& score_pair : architecture_scores) {
        if (score_pair.second > max_score) {
            max_score = score_pair.second;
            primary_type = score_pair.first;
        }
    }
    
    return primary_type;
}

std::vector<ArchitectureType> ArchitectureAnalyzer::detectSecondaryArchitectures(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<ArchitectureType> secondary_types;
    
    ArchitectureType primary = detectPrimaryArchitecture(project_metadata);
    
    if (primary != ArchitectureType::ComponentBasedEntity) {
        secondary_types.push_back(ArchitectureType::ComponentBasedEntity);
    }
    
    bool has_event_driven = false;
    for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
        if (pattern_pair.first.find("Observer") != std::string::npos) {
            has_event_driven = true;
            break;
        }
    }
    
    if (has_event_driven && primary != ArchitectureType::EventDrivenArchitecture) {
        secondary_types.push_back(ArchitectureType::EventDrivenArchitecture);
    }
    
    if (project_metadata.systems.system_groups.size() >= 3 && 
        primary != ArchitectureType::LayeredArchitecture) {
        secondary_types.push_back(ArchitectureType::LayeredArchitecture);
    }
    
    return secondary_types;
}

float ArchitectureAnalyzer::calculateArchitectureConfidence(ArchitectureType type,
                                                           const Metadata::ProjectMetadata& project_metadata) const {
    float confidence = 0.5f;
    
    auto evidence = gatherArchitectureEvidence(type, project_metadata);
    confidence += evidence.size() * 0.1f;
    
    if (type == ArchitectureType::ComponentBasedEntity) {
        confidence += 0.3f;
    }
    
    confidence += project_metadata.patterns.pattern_consistency_score * 0.2f;
    
    return std::min(1.0f, confidence);
}

std::vector<std::string> ArchitectureAnalyzer::gatherArchitectureEvidence(ArchitectureType type,
                                                                         const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> evidence;
    
    switch (type) {
        case ArchitectureType::ComponentBasedEntity:
            evidence.push_back("Unity MonoBehaviour component system");
            evidence.push_back("GameObject-based entity structure");
            if (!project_metadata.patterns.pattern_frequency.empty()) {
                evidence.push_back("Component composition patterns detected");
            }
            break;
            
        case ArchitectureType::ModelViewController:
            for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
                if (pattern_pair.first.find("MVC") != std::string::npos) {
                    evidence.push_back("MVC pattern implementation found");
                    break;
                }
            }
            break;
            
        case ArchitectureType::EntityComponentSystem:
            for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
                if (pattern_pair.first.find("ECS") != std::string::npos) {
                    evidence.push_back("ECS pattern implementation found");
                    break;
                }
            }
            break;
            
        case ArchitectureType::EventDrivenArchitecture:
            for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
                if (pattern_pair.first.find("Observer") != std::string::npos) {
                    evidence.push_back("Observer pattern for event handling");
                    break;
                }
            }
            break;
            
        case ArchitectureType::ServiceOrientedArchitecture:
            for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
                if (pattern_pair.first.find("Service") != std::string::npos) {
                    evidence.push_back("Service locator pattern detected");
                    break;
                }
            }
            break;
            
        case ArchitectureType::LayeredArchitecture:
            if (project_metadata.systems.system_groups.size() >= 3) {
                evidence.push_back("Multiple distinct system layers");
            }
            break;
            
        default:
            break;
    }
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        evidence.push_back("Contains circular dependencies");
    } else {
        evidence.push_back("Clean dependency hierarchy");
    }
    
    return evidence;
}

float ArchitectureAnalyzer::calculateSystemCohesion(const std::vector<std::string>& components,
                                                   const Metadata::ProjectMetadata& project_metadata) const {
    if (components.empty()) return 0.0f;
    
    float cohesion_score = 0.8f;
    
    std::map<std::string, int> shared_dependencies;
    for (const auto& component : components) {
        auto comp_it = project_metadata.components.find(component);
        if (comp_it != project_metadata.components.end()) {
            for (const auto& dep : comp_it->second.dependencies) {
                shared_dependencies[dep]++;
            }
        }
    }
    
    int total_shared = 0;
    for (const auto& dep_pair : shared_dependencies) {
        if (dep_pair.second > 1) {
            total_shared++;
        }
    }
    
    if (total_shared > 0) {
        cohesion_score += 0.1f;
    }
    
    return std::min(1.0f, cohesion_score);
}

float ArchitectureAnalyzer::calculateSystemCoupling(const std::vector<std::string>& components,
                                                   const Metadata::ProjectMetadata& project_metadata) const {
    if (components.empty()) return 0.0f;
    
    int external_dependencies = 0;
    int total_dependencies = 0;
    
    for (const auto& component : components) {
        auto comp_it = project_metadata.components.find(component);
        if (comp_it != project_metadata.components.end()) {
            for (const auto& dep : comp_it->second.dependencies) {
                total_dependencies++;
                
                if (std::find(components.begin(), components.end(), dep) == components.end()) {
                    external_dependencies++;
                }
            }
        }
    }
    
    if (total_dependencies == 0) return 0.0f;
    
    return static_cast<float>(external_dependencies) / total_dependencies;
}

float ArchitectureAnalyzer::assessSeparationOfConcerns(const Metadata::ProjectMetadata& project_metadata) const {
    float score = 100.0f;
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp = comp_pair.second;
        
        if (comp.responsibility_blocks.block_order.size() > 5) {
            score -= 5.0f;
        }
        
        if (comp.metrics.complexity_score > 50) {
            score -= 10.0f;
        }
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ArchitectureAnalyzer::assessDependencyInversion(const Metadata::ProjectMetadata& project_metadata) const {
    float score = 80.0f;
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        score -= 30.0f;
    }
    
    int high_coupling_count = project_metadata.dependencies.high_coupling_components.size();
    score -= high_coupling_count * 5.0f;
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ArchitectureAnalyzer::assessSingleResponsibility(const Metadata::ProjectMetadata& project_metadata) const {
    float score = 100.0f;
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp = comp_pair.second;
        
        if (comp.metrics.method_count > 15) {
            score -= 5.0f;
        }
        
        if (comp.responsibility_blocks.block_order.size() > 4) {
            score -= 3.0f;
        }
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

float ArchitectureAnalyzer::assessOpenClosed(const Metadata::ProjectMetadata& project_metadata) const {
    float score = 70.0f;
    
    int pattern_count = 0;
    for (const auto& pattern_pair : project_metadata.patterns.pattern_frequency) {
        pattern_count += pattern_pair.second;
    }
    
    if (pattern_count > 0) {
        score += pattern_count * 5.0f;
    }
    
    return std::max(0.0f, std::min(100.0f, score));
}

std::vector<std::string> ArchitectureAnalyzer::identifySOLIDViolations(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> violations;
    
    if (project_metadata.dependencies.has_circular_dependencies) {
        violations.push_back("Dependency Inversion Principle: Circular dependencies detected");
    }
    
    for (const auto& comp_pair : project_metadata.components) {
        const auto& comp = comp_pair.second;
        
        if (comp.metrics.method_count > 20) {
            violations.push_back("Single Responsibility Principle: " + comp.class_name + " has too many methods");
        }
        
        if (comp.responsibility_blocks.block_order.size() > 5) {
            violations.push_back("Single Responsibility Principle: " + comp.class_name + " has multiple responsibilities");
        }
        
        if (comp.metrics.dependency_count > 8) {
            violations.push_back("Dependency Inversion Principle: " + comp.class_name + " has too many dependencies");
        }
    }
    
    return violations;
}

std::vector<std::string> ArchitectureAnalyzer::identifyArchitecturalStrengths(const Metadata::ProjectMetadata& project_metadata) const {
    std::vector<std::string> strengths;
    
    if (!project_metadata.dependencies.has_circular_dependencies) {
        strengths.push_back("Clean dependency hierarchy without cycles");
    }
    
    if (project_metadata.systems.core_systems.size() >= 3) {
        strengths.push_back("Well-organized system architecture");
    }
    
    if (project_metadata.patterns.pattern_consistency_score > 0.7f) {
        strengths.push_back("Consistent design pattern usage");
    }
    
    if (project_metadata.quality.maintainability_score > 80.0f) {
        strengths.push_back("High maintainability score");
    }
    
    if (project_metadata.dependencies.isolated_components.size() < 2) {
        strengths.push_back("Good component integration");
    }
    
    return strengths;
}

std::string ArchitectureAnalyzer::generateArchitectureSummary(const Metadata::ProjectMetadata& project_metadata) const {
    std::stringstream summary;
    
    auto insight = analyzeArchitecture(project_metadata);
    auto health = assessArchitectureHealth(project_metadata);
    
    summary << "Architecture Analysis Summary\n";
    summary << "============================\n\n";
    
    summary << "Primary Architecture: " << getArchitectureTypeName(insight.primary_type) << "\n";
    summary << "Confidence: " << static_cast<int>(insight.confidence_score * 100) << "%\n";
    summary << "Description: " << insight.description << "\n\n";
    
    if (!insight.secondary_types.empty()) {
        summary << "Secondary Patterns:\n";
        for (const auto& type : insight.secondary_types) {
            summary << "  - " << getArchitectureTypeName(type) << "\n";
        }
        summary << "\n";
    }
    
    summary << "Architecture Health: " << static_cast<int>(health.overall_health_score) << "%\n";
    summary << "  Separation of Concerns: " << static_cast<int>(health.separation_of_concerns_score) << "%\n";
    summary << "  Dependency Management: " << static_cast<int>(health.dependency_inversion_score) << "%\n";
    summary << "  Single Responsibility: " << static_cast<int>(health.single_responsibility_score) << "%\n";
    summary << "  Extensibility: " << static_cast<int>(health.open_closed_score) << "%\n\n";
    
    if (!health.strengths.empty()) {
        summary << "Architectural Strengths:\n";
        for (const auto& strength : health.strengths) {
            summary << "  + " << strength << "\n";
        }
        summary << "\n";
    }
    
    if (!health.violations.empty()) {
        summary << "Areas for Improvement:\n";
        for (const auto& violation : health.violations) {
            summary << "  - " << violation << "\n";
        }
    }
    
    return summary.str();
}

std::string ArchitectureAnalyzer::generateImprovementPlan(const Metadata::ProjectMetadata& project_metadata) const {
    std::stringstream plan;
    
    auto health = assessArchitectureHealth(project_metadata);
    
    plan << "Architecture Improvement Plan\n";
    plan << "============================\n\n";
    
    if (health.single_responsibility_score < 70) {
        plan << "1. Improve Single Responsibility\n";
        plan << "   - Break down large components into smaller, focused ones\n";
        plan << "   - Extract utility methods into separate classes\n";
        plan << "   - Consider using composition over inheritance\n\n";
    }
    
    if (health.dependency_inversion_score < 70) {
        plan << "2. Improve Dependency Management\n";
        plan << "   - Resolve circular dependencies\n";
        plan << "   - Use dependency injection for component references\n";
        plan << "   - Consider using interfaces to reduce coupling\n\n";
    }
    
    if (health.separation_of_concerns_score < 70) {
        plan << "3. Improve Separation of Concerns\n";
        plan << "   - Separate UI logic from game logic\n";
        plan << "   - Extract data models from behavior components\n";
        plan << "   - Use events to decouple systems\n\n";
    }
    
    if (health.open_closed_score < 70) {
        plan << "4. Improve Extensibility\n";
        plan << "   - Implement strategy pattern for variable behaviors\n";
        plan << "   - Use Unity's ScriptableObject for configuration\n";
        plan << "   - Design interfaces for future extensibility\n\n";
    }
    
    plan << "5. General Recommendations\n";
    for (const auto& suggestion : project_metadata.quality.improvement_suggestions) {
        plan << "   - " << suggestion << "\n";
    }
    
    return plan.str();
}

std::string ArchitectureAnalyzer::getArchitectureTypeName(ArchitectureType type) const {
    switch (type) {
        case ArchitectureType::MonolithicComponent: return "Monolithic Component";
        case ArchitectureType::ComponentBasedEntity: return "Component-Based Entity";
        case ArchitectureType::ModelViewController: return "Model-View-Controller";
        case ArchitectureType::EntityComponentSystem: return "Entity-Component-System";
        case ArchitectureType::LayeredArchitecture: return "Layered Architecture";
        case ArchitectureType::EventDrivenArchitecture: return "Event-Driven Architecture";
        case ArchitectureType::ServiceOrientedArchitecture: return "Service-Oriented Architecture";
        case ArchitectureType::HybridArchitecture: return "Hybrid Architecture";
        default: return "Unknown Architecture";
    }
}

std::string ArchitectureAnalyzer::getArchitectureDescription(ArchitectureType type) const {
    switch (type) {
        case ArchitectureType::ComponentBasedEntity:
            return "Unity's GameObject-Component system with MonoBehaviour-based functionality";
        case ArchitectureType::ModelViewController:
            return "Separation of data (Model), presentation (View), and logic (Controller)";
        case ArchitectureType::EntityComponentSystem:
            return "Data-oriented design with entities, components, and systems";
        case ArchitectureType::EventDrivenArchitecture:
            return "Loose coupling through event-based communication";
        case ArchitectureType::LayeredArchitecture:
            return "Organized into distinct layers with clear responsibilities";
        case ArchitectureType::ServiceOrientedArchitecture:
            return "Service-based components with centralized access patterns";
        default:
            return "Standard Unity MonoBehaviour component architecture";
    }
}

} // namespace Context
} // namespace UnityContextGen