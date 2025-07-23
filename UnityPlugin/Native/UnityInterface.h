#pragma once

#include <string>

#ifdef _WIN32
#define UNITY_EXPORT __declspec(dllexport)
#define UNITY_CALL __stdcall
#else
#define UNITY_EXPORT __attribute__((visibility("default")))
#define UNITY_CALL
#endif

extern "C" {
    
    // Core analysis functions
    UNITY_EXPORT const char* UNITY_CALL AnalyzeProject(const char* projectPath, const char* optionsJson);
    UNITY_EXPORT const char* UNITY_CALL AnalyzeFile(const char* filePath, const char* optionsJson);
    
    // Context generation functions
    UNITY_EXPORT const char* UNITY_CALL GenerateLLMPrompt(const char* analysisResultJson);
    UNITY_EXPORT const char* UNITY_CALL GenerateProjectSummary(const char* analysisResultJson);
    
    // Utility functions
    UNITY_EXPORT const char* UNITY_CALL GetLastError();
    UNITY_EXPORT void UNITY_CALL FreeString(const char* str);
    UNITY_EXPORT const char* UNITY_CALL GetVersion();
    
    // Validation functions
    UNITY_EXPORT int UNITY_CALL ValidateProjectPath(const char* projectPath);
    UNITY_EXPORT int UNITY_CALL IsUnityProject(const char* projectPath);
    
    // Progress callback support
    typedef void (*ProgressCallback)(const char* message, float progress);
    UNITY_EXPORT void UNITY_CALL SetProgressCallback(ProgressCallback callback);
    
}