#pragma once

#include <string>
#include <vector>

// Test framework
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
};

extern std::vector<TestResult> g_test_results;

#define TEST(name) \
    void test_##name(); \
    struct TestRegistration_##name { \
        TestRegistration_##name() { \
            test_##name(); \
        } \
    }; \
    static TestRegistration_##name g_test_registration_##name; \
    void test_##name()

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            g_test_results.push_back({__func__, false, "Assertion failed: " #condition}); \
            return; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            g_test_results.push_back({__func__, false, "Assertion failed: " #condition " should be false"}); \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            g_test_results.push_back({__func__, false, "Assertion failed: " #expected " != " #actual}); \
            return; \
        } \
    } while(0)

#define TEST_SUCCESS() \
    g_test_results.push_back({__func__, true, "Test passed"})