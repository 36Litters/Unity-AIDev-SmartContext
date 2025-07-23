# Unity Plugin Native Library CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(UnityLLMContextPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build shared library (DLL) for Unity
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Unity Interface sources
set(UNITY_INTERFACE_SOURCES
    UnityInterface.cpp
    UnityInterface.h
)

# Create shared library for Unity
add_library(unity_context_core_plugin SHARED ${UNITY_INTERFACE_SOURCES})

# Link to the core library
target_link_libraries(unity_context_core_plugin
    unity_context_core
    ${TREE_SITTER_LIB}
    ${TREE_SITTER_CSHARP_LIB}
    nlohmann_json::nlohmann_json
)

# Include core headers
target_include_directories(unity_context_core_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Core/TreeSitterEngine
    ${CMAKE_SOURCE_DIR}/Core/UnityAnalyzer
    ${CMAKE_SOURCE_DIR}/Core/MetadataGenerator
    ${CMAKE_SOURCE_DIR}/Core/ContextSummarizer
)

# Set output name to match Unity's expectations
set_target_properties(unity_context_core_plugin PROPERTIES
    OUTPUT_NAME "unity_context_core"
    PREFIX ""
)

# Platform-specific settings
if(WIN32)
    # Windows DLL settings
    set_target_properties(unity_context_core_plugin PROPERTIES
        SUFFIX ".dll"
    )
    
    # Copy DLL to Unity plugin directory after build
    add_custom_command(TARGET unity_context_core_plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:unity_context_core_plugin>
        ${CMAKE_CURRENT_SOURCE_DIR}/x64/unity_context_core.dll
        COMMENT "Copying DLL to Unity plugin directory"
    )
elseif(APPLE)
    # macOS settings
    set_target_properties(unity_context_core_plugin PROPERTIES
        SUFFIX ".dylib"
    )
    
    add_custom_command(TARGET unity_context_core_plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:unity_context_core_plugin>
        ${CMAKE_CURRENT_SOURCE_DIR}/x64/unity_context_core.dylib
        COMMENT "Copying dylib to Unity plugin directory"
    )
else()
    # Linux settings
    set_target_properties(unity_context_core_plugin PROPERTIES
        SUFFIX ".so"
    )
    
    add_custom_command(TARGET unity_context_core_plugin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:unity_context_core_plugin>
        ${CMAKE_CURRENT_SOURCE_DIR}/x64/unity_context_core.so
        COMMENT "Copying so to Unity plugin directory"
    )
endif()

# Install target
install(TARGETS unity_context_core_plugin
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/x64
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/x64
    ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/x64
)