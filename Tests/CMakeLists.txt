# Tests CMakeLists.txt

# Enable testing
enable_testing()

# Test sources
set(TEST_SOURCES
    test_main.cpp
    test_treesitter_engine.cpp
    test_unity_analyzer.cpp
    test_metadata_generator.cpp
    test_core_engine.cpp
)

# Sample Unity C# files for testing
set(SAMPLE_FILES
    SampleUnityProject/PlayerController.cs
    SampleUnityProject/EnemyAI.cs
    SampleUnityProject/GameManager.cs
    SampleUnityProject/UIController.cs
)

# Create test executable
add_executable(unity_context_tests ${TEST_SOURCES})

# Link with core library
target_link_libraries(unity_context_tests
    unity_context_core
    ${TREE_SITTER_LIB}
    nlohmann_json::nlohmann_json
)

# Headers
target_include_directories(unity_context_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
)

# Copy sample files to build directory
foreach(SAMPLE_FILE ${SAMPLE_FILES})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${SAMPLE_FILE}
        COPYONLY
    )
endforeach()

# Add test
add_test(NAME unity_context_tests COMMAND unity_context_tests)