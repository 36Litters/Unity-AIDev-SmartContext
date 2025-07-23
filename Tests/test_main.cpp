#include <iostream>
#include "test_framework.h"

// Test framework global variable definition
std::vector<TestResult> g_test_results;

// Forward declarations
void test_treesitter_engine();
void test_unity_analyzer();
void test_metadata_generator();
void test_core_engine();

int main() {
    std::cout << "Running Unity Context Generator Tests\n";
    std::cout << "====================================\n\n";
    
    // Run all tests
    test_treesitter_engine();
    test_unity_analyzer();
    test_metadata_generator();
    test_core_engine();
    
    // Print results
    int passed = 0;
    int failed = 0;
    
    for (const auto& result : g_test_results) {
        if (result.passed) {
            std::cout << "✅ " << result.name << ": " << result.message << "\n";
            passed++;
        } else {
            std::cout << "❌ " << result.name << ": " << result.message << "\n";
            failed++;
        }
    }
    
    std::cout << "\n====================================\n";
    std::cout << "Tests completed: " << (passed + failed) << "\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    
    return failed == 0 ? 0 : 1;
}