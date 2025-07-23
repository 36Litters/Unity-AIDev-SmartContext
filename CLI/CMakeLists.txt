# CLI executable CMakeLists.txt

set(CLI_SOURCES
    main.cpp
    CLIArguments.cpp
    CLIApplication.cpp
)

# Create CLI executable
add_executable(unity_context_generator ${CLI_SOURCES})

# Link with core library
target_link_libraries(unity_context_generator
    unity_context_core
    ${TREE_SITTER_LIB}
    nlohmann_json::nlohmann_json
)

# Headers
target_include_directories(unity_context_generator PRIVATE
    ${CMAKE_SOURCE_DIR}/Core
)

# Installation
install(TARGETS unity_context_generator
    RUNTIME DESTINATION bin
)