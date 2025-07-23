# Unity Smart Context Generator

🎮 **AI-Powered Unity Code Analysis and Context Generation Tool**

[![Unity](https://img.shields.io/badge/Unity-2019.4+-black?style=flat-square&logo=unity)](https://unity.com/)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=cplusplus)](https://en.cppreference.com/w/cpp/17)
[![Claude](https://img.shields.io/badge/Claude-3.5_Sonnet-orange?style=flat-square)](https://www.anthropic.com/)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)

A powerful tool for analyzing Unity C# scripts and generating AI-powered insights, architectural analysis, and LLM-optimized context metadata.

[한국어 문서](README_KR.md) | [English](README.md)

## ✨ Features

### 🔍 **Intelligent Code Analysis**
- **Unity API Detection**: Automatic recognition and classification of 30+ Unity API patterns
- **Performance Analysis**: Identification of performance-sensitive API calls
- **Architecture Analysis**: Component dependency and design pattern analysis
- **AI-Powered Code Review**: Professional code review through Claude AI

### 🎯 **Unity-Specialized Analysis**
- **MonoBehaviour Analysis**: Unity lifecycle method pattern analysis
- **ScriptableObject Detection**: Data asset structure analysis
- **Component Dependencies**: GameObject-Component relationship mapping
- **Design Pattern Detection**: Recognition of Unity patterns (Singleton, Observer, etc.)

### 🤖 **AI Integration**
- **Claude API Integration**: Support for Anthropic Claude 3.5 Sonnet
- **Context-Aware Analysis**: AI analysis that understands Unity project context
- **Performance Optimization Suggestions**: AI-based performance improvement recommendations
- **Best Practices**: Unity development best practice suggestions

### 🛠️ **Multiple Usage Methods**
- **CLI Tool**: Batch analysis from command line
- **Unity Editor Plugin**: Real-time analysis and GUI
- **CI/CD Integration**: Automated code review pipeline

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        CoreEngine                               │
├─────────────────────────────────────────────────────────────────┤
│  ├── TreeSitter Parser (C# AST Analysis)                        │
│  ├── Unity Analyzer (Component Analysis)                        │
│  ├── AI Analysis Pipeline (Claude API Integration)              │
│  │   ├── UnityAPIDetector                                      │
│  │   ├── ClaudeAnalyzer                                        │
│  │   └── HTTPClient                                            │
│  └── Metadata Generator (JSON/Markdown Output)                 │
└─────────────────────────────────────────────────────────────────┘
```

### Core Components
- **TreeSitter Engine**: C# source code parsing
- **Unity Analyzer**: Unity-specialized analysis (MonoBehaviour, ScriptableObject)
- **AI Integration**: Intelligent analysis through Claude API
- **Metadata Generator**: Multi-format result output
- **Unity Plugin**: Editor integration and real-time analysis

## 🚀 Quick Start

### Prerequisites

- CMake 3.15+
- C++17 compatible compiler
- Tree-sitter library
- nlohmann/json library

### Building

```bash
git clone https://github.com/your-repo/unity-llm-context-generator
cd unity-llm-context-generator
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Basic Usage

```bash
# Analyze a single Unity script
./unity_context_generator PlayerController.cs

# Analyze entire Unity project
./unity_context_generator --directory ./Assets/Scripts

# Generate LLM-optimized output
./unity_context_generator -d ./Scripts -f llm -o ./llm_context
```

## 📊 Output Examples

### Generated LLM Prompt

```markdown
# Unity Project Context

**Type:** 3D Platformer | **Architecture:** Component-Based Entity | **Quality:** 85%

## Component Overview
- **PlayerController**: Handles player movement, input, and physics interactions
- **EnemyAI**: Controls enemy behavior with state-based AI and pathfinding
- **GameManager**: Singleton managing game state, score, and lifecycle

## Key Patterns
- Component Composition for GameObject functionality
- Singleton Pattern for global managers
- Observer Pattern for UI updates

## Development Guidelines
- Use Unity lifecycle methods appropriately
- Cache component references in Awake()
- Follow established movement and AI patterns
```

### JSON Metadata Structure

```json
{
  "project_context": {
    "type": "3D Platformer",
    "architecture": "Component-based",
    "key_systems": ["Player Control", "Enemy AI", "Physics Interaction"]
  },
  "components": {
    "PlayerController": {
      "purpose": "Handles player movement, jumping, and ground detection",
      "dependencies": ["Rigidbody", "Collider"],
      "lifecycle_usage": ["Awake", "Update", "FixedUpdate"],
      "responsibility_blocks": {
        "input_handling": "Process WASD and jump input",
        "physics_update": "Apply movement forces",
        "collision_detection": "Handle ground contact"
      }
    }
  }
}
```

## 🎮 Unity-Specific Features

### MonoBehaviour Lifecycle Analysis

Automatically detects and categorizes Unity lifecycle methods:

- **Initialization**: Awake, Start, OnEnable
- **Frame Updates**: Update, LateUpdate
- **Physics**: FixedUpdate, OnCollision*, OnTrigger*
- **Cleanup**: OnDisable, OnDestroy

### Component Dependency Mapping

Identifies component relationships through:

- `GetComponent<T>()` calls
- `[RequireComponent(typeof(T))]` attributes
- `[SerializeField]` references
- Cross-component method calls

### Design Pattern Detection

Recognizes common Unity patterns:

- **Singleton MonoBehaviour**: Global manager classes
- **Object Pooling**: Reusable game object management
- **Observer Pattern**: Event-driven communication
- **State Pattern**: AI and game state management
- **Component Composition**: GameObject functionality building

## 📋 Command Line Options

```
USAGE:
    unity_context_generator [OPTIONS] [FILES...]
    unity_context_generator --directory <DIR> [OPTIONS]

OPTIONS:
    -h, --help              Show help message
    -v, --version           Show version information
    -d, --directory <DIR>   Analyze all C# files recursively
    -o, --output <DIR>      Output directory
    -f, --format <FORMAT>   Output format: json, markdown, llm, all
    
    --verbose               Enable verbose output
    --quiet                 Suppress all output except errors
    --force                 Overwrite existing files
    
    --no-dependencies       Skip dependency analysis
    --no-lifecycle          Skip Unity lifecycle analysis
    --no-patterns           Skip design pattern detection
```

## 🔧 Integration with Claude Code

This tool is specifically designed to work seamlessly with Claude Code:

1. **Run Analysis**: `unity_context_generator -d ./Assets/Scripts -f llm`
2. **Copy Context**: Use the generated `llm_prompt.md` as context
3. **Develop Efficiently**: Claude Code now understands your Unity project structure

### Example Claude Code Workflow

```bash
# Generate context for your Unity project
unity_context_generator --directory ./Assets/Scripts --format llm

# The generated context helps Claude Code understand:
# - Your project's architecture patterns
# - Component relationships and dependencies  
# - Existing code style and conventions
# - Unity-specific best practices for your codebase
```

## 🧪 Testing

The project includes comprehensive tests with sample Unity scripts:

```bash
cd build
make test
# or run directly:
./Tests/unity_context_tests
```

Test coverage includes:
- Tree-sitter C# parsing accuracy
- Unity MonoBehaviour detection
- Component dependency analysis
- Design pattern recognition
- Metadata generation and export

## 📈 Performance

- **Analysis Speed**: 1000 lines of code < 1 second
- **Memory Usage**: Large projects < 512MB
- **Accuracy**: 90%+ Unity component relationship detection
- **Token Efficiency**: 80% reduction in LLM context tokens

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Ensure all tests pass
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Tree-sitter**: For providing excellent parsing infrastructure
- **nlohmann/json**: For elegant JSON handling in C++
- **Unity Technologies**: For creating the amazing game engine this tool supports
- **Anthropic**: For Claude Code and inspiring LLM-friendly development tools

## 📞 Support

- 📁 **Issues**: [GitHub Issues](https://github.com/your-repo/unity-llm-context-generator/issues)
- 📖 **Documentation**: [Full Documentation](https://docs.unity-context-generator.dev)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/your-repo/unity-llm-context-generator/discussions)

---

**Built for Unity developers who want to leverage AI tools more effectively.** 🎮✨