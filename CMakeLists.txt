cmake_minimum_required(VERSION 3.15)
project(UnityLLMContextGenerator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find required packages
find_package(PkgConfig REQUIRED)

# Tree-sitter
find_library(TREE_SITTER_LIB tree-sitter)
if(NOT TREE_SITTER_LIB)
    message(FATAL_ERROR "Tree-sitter library not found")
endif()

# Tree-sitter C# (local library)
set(TREE_SITTER_CSHARP_LIB ${CMAKE_SOURCE_DIR}/libtree-sitter-c-sharp.a)

# JSON library (nlohmann/json)
find_package(nlohmann_json REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Core/TreeSitterEngine
    ${CMAKE_SOURCE_DIR}/Core/UnityAnalyzer
    ${CMAKE_SOURCE_DIR}/Core/MetadataGenerator
    ${CMAKE_SOURCE_DIR}/Core/ContextSummarizer
)

# Core library
add_subdirectory(Core)

# CLI executable
add_subdirectory(CLI)

# Tests
enable_testing()
add_subdirectory(Tests)

# Unity Plugin (optional)
option(BUILD_UNITY_PLUGIN "Build Unity plugin DLL" ON)
if(BUILD_UNITY_PLUGIN)
    add_subdirectory(UnityPlugin/Native)
endif()

# Installation
install(TARGETS unity_context_generator
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)