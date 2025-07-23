#pragma once

#include "../MetadataGenerator/ProjectMetadata.h"
#include <string>
#include <vector>
#include <map>

namespace UnityContextGen {
namespace Context {

enum class ArchitectureType {
    MonolithicComponent,
    ComponentBasedEntity,
    ModelViewController,
    EntityComponentSystem,
    LayeredArchitecture,
    EventDrivenArchitecture,
    ServiceOrientedArchitecture,
    HybridArchitecture,
    Unknown
};

struct ArchitectureInsight {
    ArchitectureType primary_type;
    std::vector<ArchitectureType> secondary_types;
    float confidence_score;
    std::vector<std::string> evidence;
    std::vector<std::string> characteristics;
    std::string description;
};

struct SystemCohesion {
    std::string system_name;
    std::vector<std::string> components;
    float cohesion_score;
    float coupling_score;
    std::string responsibility;
    bool is_well_designed;
};

struct ArchitectureHealth {
    float separation_of_concerns_score;
    float dependency_inversion_score;
    float single_responsibility_score;
    float open_closed_score;
    float overall_health_score;
    std::vector<std::string> violations;
    std::vector<std::string> strengths;
};

class ArchitectureAnalyzer {
public:
    ArchitectureAnalyzer();
    ~ArchitectureAnalyzer() = default;

    ArchitectureInsight analyzeArchitecture(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<SystemCohesion> analyzeSystemCohesion(const Metadata::ProjectMetadata& project_metadata) const;
    ArchitectureHealth assessArchitectureHealth(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::string generateArchitectureSummary(const Metadata::ProjectMetadata& project_metadata) const;
    std::string generateImprovementPlan(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::vector<std::string> identifyArchitecturalPrinciples(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> detectAntiPatterns(const Metadata::ProjectMetadata& project_metadata) const;

private:
    ArchitectureType detectPrimaryArchitecture(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<ArchitectureType> detectSecondaryArchitectures(const Metadata::ProjectMetadata& project_metadata) const;
    
    float calculateArchitectureConfidence(ArchitectureType type, 
                                         const Metadata::ProjectMetadata& project_metadata) const;
    
    std::vector<std::string> gatherArchitectureEvidence(ArchitectureType type,
                                                        const Metadata::ProjectMetadata& project_metadata) const;
    
    float calculateSystemCohesion(const std::vector<std::string>& components,
                                 const Metadata::ProjectMetadata& project_metadata) const;
    
    float calculateSystemCoupling(const std::vector<std::string>& components,
                                 const Metadata::ProjectMetadata& project_metadata) const;
    
    float assessSeparationOfConcerns(const Metadata::ProjectMetadata& project_metadata) const;
    float assessDependencyInversion(const Metadata::ProjectMetadata& project_metadata) const;
    float assessSingleResponsibility(const Metadata::ProjectMetadata& project_metadata) const;
    float assessOpenClosed(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::vector<std::string> identifySOLIDViolations(const Metadata::ProjectMetadata& project_metadata) const;
    std::vector<std::string> identifyArchitecturalStrengths(const Metadata::ProjectMetadata& project_metadata) const;
    
    std::string getArchitectureTypeName(ArchitectureType type) const;
    std::string getArchitectureDescription(ArchitectureType type) const;
};

} // namespace Context
} // namespace UnityContextGen