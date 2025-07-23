# Core library CMakeLists.txt

# TreeSitterEngine sources
set(TREE_SITTER_ENGINE_SOURCES
    TreeSitterEngine/TreeSitterWrapper.cpp
    TreeSitterEngine/CSharpParser.cpp
    TreeSitterEngine/ASTNode.cpp
)

# UnityAnalyzer sources  
set(UNITY_ANALYZER_SOURCES
    UnityAnalyzer/MonoBehaviourAnalyzer.cpp
    UnityAnalyzer/ComponentDependencyAnalyzer.cpp
    UnityAnalyzer/LifecycleAnalyzer.cpp
    UnityAnalyzer/UnityPatternDetector.cpp
    UnityAnalyzer/ScriptableObjectAnalyzer.cpp
)

# MetadataGenerator sources
set(METADATA_GENERATOR_SOURCES
    MetadataGenerator/ComponentMetadata.cpp
    MetadataGenerator/ProjectMetadata.cpp
    MetadataGenerator/JSONExporter.cpp
)

# ContextSummarizer sources
set(CONTEXT_SUMMARIZER_SOURCES
    ContextSummarizer/ArchitectureAnalyzer.cpp
    ContextSummarizer/DataFlowAnalyzer.cpp
    ContextSummarizer/ProjectSummarizer.cpp
)

# AI Integration sources
set(AI_SOURCES
    AI/UnityAPIDetector.cpp
    AI/ClaudeAnalyzer.cpp
    AI/HTTPClient.cpp
)

# Configuration sources  
set(CONFIGURATION_SOURCES
    Configuration/AnalysisConfig.cpp
)

# Main engine sources
set(CORE_SOURCES
    CoreEngine.cpp
    ${TREE_SITTER_ENGINE_SOURCES}
    ${UNITY_ANALYZER_SOURCES}
    ${METADATA_GENERATOR_SOURCES}
    ${CONTEXT_SUMMARIZER_SOURCES}
    ${AI_SOURCES}
    ${CONFIGURATION_SOURCES}
)

# Create core library
add_library(unity_context_core STATIC ${CORE_SOURCES})

# Enable PIC for shared library compatibility
set_target_properties(unity_context_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Find additional libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Link libraries
target_link_libraries(unity_context_core
    ${TREE_SITTER_LIB}
    ${TREE_SITTER_CSHARP_LIB}
    nlohmann_json::nlohmann_json
    ${CURL_LIBRARIES}
)

# Include directories
target_include_directories(unity_context_core PRIVATE
    ${CURL_INCLUDE_DIRS}
)

# Headers
target_include_directories(unity_context_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)