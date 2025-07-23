#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <string>

// Function signatures matching UnityInterface.h
typedef const char* (*AnalyzeProjectFunc)(const char*, const char*);
typedef const char* (*GenerateLLMPromptFunc)(const char*);
typedef const char* (*GetLastErrorFunc)();
typedef void (*FreeStringFunc)(const char*);
typedef const char* (*GetVersionFunc)();

int main() {
    std::cout << "Testing Unity Plugin DLL..." << std::endl;
    
    // Load the shared library
    void* handle = dlopen("./UnityPlugin/Native/x64/unity_context_core.so", RTLD_LAZY);
    if (!handle) {
        std::cerr << "Error loading library: " << dlerror() << std::endl;
        return 1;
    }
    
    // Load functions
    auto AnalyzeProject = (AnalyzeProjectFunc)dlsym(handle, "AnalyzeProject");
    auto GenerateLLMPrompt = (GenerateLLMPromptFunc)dlsym(handle, "GenerateLLMPrompt");
    auto GetLastError = (GetLastErrorFunc)dlsym(handle, "GetLastError");
    auto FreeString = (FreeStringFunc)dlsym(handle, "FreeString");
    auto GetVersion = (GetVersionFunc)dlsym(handle, "GetVersion");
    
    if (!AnalyzeProject || !GenerateLLMPrompt || !GetLastError || !FreeString || !GetVersion) {
        std::cerr << "Error loading functions: " << dlerror() << std::endl;
        dlclose(handle);
        return 1;
    }
    
    std::cout << "✓ DLL loaded successfully!" << std::endl;
    
    // Test GetVersion
    const char* version = GetVersion();
    if (version) {
        std::cout << "✓ Plugin version: " << version << std::endl;
        FreeString(version);
    }
    
    // Test analysis with our sample files
    const char* options = R"({
        "analyzeDependencies": true,
        "analyzeLifecycleMethods": true,
        "detectDesignPatterns": true,
        "generateMetadata": true,
        "generateContextSummary": true,
        "exportJson": true,
        "verboseLogging": true,
        "outputDirectory": "./test_output"
    })";
    
    std::cout << "\nTesting project analysis..." << std::endl;
    const char* result = AnalyzeProject("./TestSamples", options);
    
    if (result) {
        std::cout << "✓ Analysis successful!" << std::endl;
        std::cout << "\nAnalysis Result (first 500 chars):" << std::endl;
        std::string resultStr(result);
        if (resultStr.length() > 500) {
            std::cout << resultStr.substr(0, 500) << "..." << std::endl;
        } else {
            std::cout << resultStr << std::endl;
        }
        
        // Test LLM prompt generation
        std::cout << "\nTesting LLM prompt generation..." << std::endl;
        const char* prompt = GenerateLLMPrompt(result);
        if (prompt) {
            std::cout << "✓ Prompt generated!" << std::endl;
            std::cout << "\nGenerated Prompt:" << std::endl;
            std::cout << prompt << std::endl;
            FreeString(prompt);
        } else {
            std::cout << "✗ Prompt generation failed" << std::endl;
        }
        
        FreeString(result);
    } else {
        std::cout << "✗ Analysis failed" << std::endl;
        const char* error = GetLastError();
        if (error) {
            std::cout << "Error: " << error << std::endl;
            FreeString(error);
        }
    }
    
    std::cout << "\n✓ Plugin test completed!" << std::endl;
    
    dlclose(handle);
    return 0;
}