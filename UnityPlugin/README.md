# Unity LLM Context Generator Plugin

A Unity Package Manager compatible plugin that provides LLM-friendly context generation for Unity projects.

## Features

- **MonoBehaviour Analysis**: Automatically detects and analyzes Unity MonoBehaviour components
- **Dependency Mapping**: Tracks component dependencies and relationships
- **Lifecycle Analysis**: Identifies Unity lifecycle method usage patterns
- **Design Pattern Detection**: Recognizes common Unity design patterns
- **LLM Prompt Generation**: Creates context-aware prompts for AI assistance
- **Unity Editor Integration**: Easy-to-use GUI integrated into Unity's Editor

## Installation

### Option 1: Package Manager (Recommended)
1. Copy this entire `UnityPlugin` directory to your project's `Packages` folder
2. Rename it to `com.unity.llm-context-generator`
3. Unity will automatically import the package

### Option 2: Manual Installation
1. Copy the contents to your project's `Assets` directory
2. Unity will compile and integrate the plugin automatically

## Usage

### From Unity Editor
1. Open Unity Editor
2. Navigate to **Tools** → **LLM Context Generator**
3. Select your project's Assets directory
4. Configure analysis options
5. Click **Analyze Project**
6. Export or copy the generated LLM prompt

### From C# Scripts
```csharp
using UnityEngine;

public class ExampleUsage : MonoBehaviour
{
    void Start()
    {
        // Analyze current project
        var result = LLMContextAnalyzer.AnalyzeCurrentProject();
        
        if (result.Success)
        {
            Debug.Log($"Found {result.MonoBehaviourCount} components");
            
            // Generate LLM prompt
            string prompt = LLMContextAnalyzer.GeneratePromptFromResult(result);
            Debug.Log(prompt);
        }
    }
}
```

## Requirements

- Unity 2019.4 LTS or newer
- .NET Framework 4.x or .NET Standard 2.0
- Windows, macOS, or Linux development environment

## Architecture

### C# Components
- **LLMContextAnalyzer**: Main interface for project analysis
- **LLMContextGeneratorWindow**: Unity Editor GUI
- **AnalysisDataTypes**: Data structures for analysis results

### Native C++ Backend
- **CoreEngine**: Main analysis engine
- **TreeSitterEngine**: C# AST parsing using Tree-sitter
- **UnityAnalyzer**: Unity-specific analysis components
- **MetadataGenerator**: Project metadata extraction
- **ContextSummarizer**: LLM context generation

### Package Structure
```
com.unity.llm-context-generator/
├── package.json                 # Package Manager manifest
├── Runtime/
│   ├── Scripts/
│   │   ├── LLMContextAnalyzer.cs
│   │   └── AnalysisDataTypes.cs
│   └── Unity.LLMContext.Runtime.asmdef
├── Editor/
│   ├── Scripts/
│   │   └── LLMContextGeneratorWindow.cs
│   └── Unity.LLMContext.Editor.asmdef
├── Native/
│   ├── x64/
│   │   └── unity_context_core.so/.dll/.dylib
│   └── UnityInterface.h
└── Documentation~/
    └── README.md
```

## Building from Source

### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- Tree-sitter library
- nlohmann/json library

### Build Steps
1. Run the build script: `./build_plugin.sh`
2. Or manually build with CMake:
   ```bash
   mkdir build && cd build
   cmake .. -DBUILD_UNITY_PLUGIN=ON
   make unity_context_core_plugin
   ```

## Troubleshooting

### Common Issues

**Plugin not loading in Unity**
- Ensure the native library matches your platform (Windows: .dll, macOS: .dylib, Linux: .so)
- Check Unity Console for P/Invoke errors
- Verify Unity's API Compatibility Level matches the plugin

**Analysis not working**
- Ensure your project has C# scripts in the Assets directory
- Check that Tree-sitter library dependencies are available
- Enable verbose logging in the analysis options

**Build errors**
- Install required dependencies: Tree-sitter, nlohmann/json
- Ensure CMake 3.15+ is installed
- Check compiler C++17 support

### Debug Information

Enable verbose logging in the Unity Editor window to get detailed analysis information:
- Component detection progress
- Dependency resolution steps
- Pattern detection results
- Error messages and warnings

## Performance

- **Analysis Speed**: ~100-500 C# files per second
- **Memory Usage**: ~50-200MB depending on project size
- **Incremental Analysis**: Supports single file analysis for faster iterations

## License

This project is part of the Unity LLM Context Generator toolkit.

## Support

For issues, feature requests, or contributions, please refer to the main project repository.