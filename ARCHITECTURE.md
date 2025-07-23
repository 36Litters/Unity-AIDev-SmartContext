# Unity LLM Context Generator - 아키텍처 문서

## 🏗️ 전체 시스템 아키텍처

### 📋 **아키텍처 원칙**
1. **모듈성**: 각 컴포넌트는 독립적으로 테스트 가능
2. **확장성**: 새로운 분석 기능 쉽게 추가 가능
3. **성능**: Tree-sitter 기반 고속 파싱
4. **LLM 최적화**: 토큰 효율성을 위한 구조화된 출력

### 🎯 **핵심 설계 목표**
- Unity C# 코드의 의미론적 분석
- LLM 친화적 메타데이터 생성
- 실시간 분석 지원
- 크로스 플랫폼 호환성

## 📦 **모듈 구조**

```
UnityLLMContextGenerator/
├── Core/                          # C++ 핵심 엔진
│   ├── TreeSitterEngine/         # 파싱 레이어
│   ├── UnityAnalyzer/            # Unity 특화 분석
│   ├── MetadataGenerator/        # 메타데이터 생성
│   ├── ContextSummarizer/        # 컨텍스트 요약
│   └── CoreEngine.cpp/h          # 메인 API
├── UnityPlugin/                  # Unity Editor 연동 (계획)
├── CLI/                          # 명령줄 도구
├── Tests/                        # 테스트 시스템
└── Documentation/                # 문서
```

## 🔧 **Core Engine 아키텍처**

### 1. **TreeSitterEngine 모듈**

```cpp
namespace UnityContextGen::TreeSitter {
    class TreeSitterWrapper    // Tree-sitter C API 래퍼
    class CSharpParser        // 고수준 C# 파싱 인터페이스  
    class ASTNode            // AST 노드 조작 및 탐색
}
```

#### **책임**
- C# 소스 코드의 구문 분석
- AST(Abstract Syntax Tree) 생성 및 탐색
- 파싱 에러 처리 및 복구
- Unity 특화 C# 구문 지원

#### **핵심 클래스**

**TreeSitterWrapper**
```cpp
class TreeSitterWrapper {
private:
    TSParser* m_parser;          // Tree-sitter 파서
    TSTree* m_tree;              // 파싱 결과 트리
    std::string m_source_code;   // 원본 소스 코드

public:
    bool parseCode(const std::string& source_code);
    std::unique_ptr<ASTNode> getRootNode() const;
    bool hasParseErrors() const;
};
```

**CSharpParser**
```cpp
class CSharpParser {
private:
    TreeSitterWrapper m_wrapper;
    std::vector<ClassInfo> m_classes;
    std::vector<MethodInfo> m_methods;
    std::vector<FieldInfo> m_fields;

public:
    bool parseFile(const std::string& file_path);
    std::vector<ClassInfo> getClasses() const;
    std::vector<MethodInfo> getMethods() const;
    std::vector<FieldInfo> getFields() const;
};
```

### 2. **UnityAnalyzer 모듈**

```cpp
namespace UnityContextGen::Unity {
    class MonoBehaviourAnalyzer        // MonoBehaviour 분석
    class ComponentDependencyAnalyzer  // 컴포넌트 의존성 분석
    class LifecycleAnalyzer           // Unity 생명주기 분석
    class UnityPatternDetector        // 디자인 패턴 감지
}
```

#### **MonoBehaviourAnalyzer 세부 구조**

```cpp
struct MonoBehaviourInfo {
    std::string class_name;                    // 클래스 이름
    std::string file_path;                     // 파일 경로
    std::vector<std::string> unity_methods;    // Unity 메서드 목록
    std::vector<std::string> custom_methods;   // 커스텀 메서드 목록
    std::vector<std::string> serialized_fields; // 직렬화된 필드
    std::vector<std::string> component_dependencies; // 컴포넌트 의존성
    std::map<std::string, std::string> method_purposes; // 메서드 목적
};

class MonoBehaviourAnalyzer {
private:
    static const std::set<std::string> UNITY_LIFECYCLE_METHODS;
    static const std::map<std::string, std::string> METHOD_EXECUTION_PHASES;

public:
    bool analyzeFile(const std::string& file_path);
    std::vector<MonoBehaviourInfo> getMonoBehaviours() const;
    bool isUnityMethod(const std::string& method_name) const;
    std::string getMethodExecutionPhase(const std::string& method_name) const;
};
```

#### **ComponentDependencyAnalyzer 구조**

```cpp
struct ComponentDependency {
    std::string source_component;    // 의존성 소스
    std::string target_component;    // 의존성 타겟
    std::string dependency_type;     // 의존성 타입 ("GetComponent", "RequireComponent")
    std::string method_context;      // 사용 컨텍스트
    size_t line_number;             // 라인 번호
};

struct ComponentGraph {
    std::map<std::string, std::vector<std::string>> dependencies;  // 의존성 그래프
    std::map<std::string, std::vector<std::string>> dependents;    // 역방향 의존성
    std::vector<ComponentDependency> edges;                        // 의존성 엣지
};
```

### 3. **MetadataGenerator 모듈**

```cpp
namespace UnityContextGen::Metadata {
    class ComponentMetadataGenerator   // 컴포넌트 메타데이터
    class ProjectMetadataGenerator    // 프로젝트 메타데이터  
    class JSONExporter               // JSON 출력
}
```

#### **메타데이터 구조**

```cpp
struct ComponentMetadata {
    std::string class_name;           // 컴포넌트 이름
    std::string file_path;           // 파일 경로
    std::string purpose;             // 컴포넌트 목적
    std::vector<std::string> dependencies; // 의존성 목록
    
    struct LifecycleInfo {
        std::vector<std::string> methods;              // 생명주기 메서드
        std::map<std::string, std::string> method_purposes; // 메서드별 목적
        std::vector<std::string> execution_order;      // 실행 순서
        std::vector<std::string> data_flow;           // 데이터 플로우
    } lifecycle;
    
    struct ResponsibilityBlocks {
        std::map<std::string, std::string> blocks;     // 책임 블록
        std::vector<std::string> block_order;         // 블록 순서
    } responsibility_blocks;
    
    struct MetricsInfo {
        int complexity_score;         // 복잡도 점수
        int dependency_count;         // 의존성 개수
        int method_count;            // 메서드 개수
        float pattern_confidence;    // 패턴 신뢰도
    } metrics;
};
```

### 4. **ContextSummarizer 모듈**

```cpp
namespace UnityContextGen::Context {
    class ArchitectureAnalyzer    // 아키텍처 분석
    class DataFlowAnalyzer       // 데이터 플로우 분석
    class ProjectSummarizer      // 프로젝트 요약
}
```

#### **아키텍처 분석 구조**

```cpp
enum class ArchitectureType {
    MonolithicComponent,      // 단일체 컴포넌트
    ComponentBasedEntity,     // 컴포넌트 기반 엔티티
    ModelViewController,      // MVC 패턴
    EntityComponentSystem,    // ECS 패턴
    EventDrivenArchitecture, // 이벤트 기반
    ServiceOrientedArchitecture, // 서비스 지향
    HybridArchitecture       // 하이브리드
};

struct ArchitectureInsight {
    ArchitectureType primary_type;              // 주요 아키텍처 타입
    std::vector<ArchitectureType> secondary_types; // 보조 타입들
    float confidence_score;                     // 신뢰도 점수
    std::vector<std::string> evidence;          // 증거 목록
    std::vector<std::string> characteristics;   // 특성 목록
    std::string description;                    // 설명
};
```

## 🔄 **데이터 플로우**

### **1. 입력 단계**
```
Unity C# Files → TreeSitterEngine → AST Generation
```

### **2. 분석 단계**
```
AST → UnityAnalyzer → Component Analysis
                   → Dependency Mapping  
                   → Lifecycle Analysis
                   → Pattern Detection
```

### **3. 메타데이터 생성**
```
Analysis Results → MetadataGenerator → Structured Metadata
                                   → Quality Metrics
                                   → Component Purposes
```

### **4. 컨텍스트 요약**
```
Metadata → ContextSummarizer → Architecture Insights
                            → Data Flow Analysis  
                            → LLM-Optimized Context
```

### **5. 출력 단계**
```
Context → JSONExporter → project_metadata.json
                      → llm_optimized.json
                      → llm_prompt.md
                      → detailed_report.md
```

## ⚡ **성능 최적화 전략**

### **1. 파싱 최적화**
- Tree-sitter incremental parsing 활용
- AST 캐싱 시스템
- 메모리 효율적인 노드 탐색

### **2. 분석 최적화**
- 병렬 파일 처리
- 의존성 그래프 최적화
- 패턴 매칭 알고리즘 최적화

### **3. 메모리 관리**
- RAII 패턴 활용
- 스마트 포인터 사용
- 메모리 풀링 (대용량 프로젝트용)

## 🔌 **확장성 설계**

### **1. 플러그인 아키텍처**
```cpp
class AnalysisPlugin {
public:
    virtual bool analyze(const AST& ast) = 0;
    virtual MetadataContribution getMetadata() = 0;
    virtual std::string getName() const = 0;
};

class PluginManager {
    std::vector<std::unique_ptr<AnalysisPlugin>> m_plugins;
public:
    void registerPlugin(std::unique_ptr<AnalysisPlugin> plugin);
    void analyzeWithPlugins(const AST& ast);
};
```

### **2. 설정 시스템**
```cpp
struct AnalysisConfiguration {
    bool enable_dependency_analysis = true;
    bool enable_pattern_detection = true;
    bool enable_lifecycle_analysis = true;
    std::vector<std::string> custom_unity_methods;
    std::map<std::string, float> pattern_confidence_thresholds;
};
```

### **3. 출력 포맷 확장**
```cpp
class OutputFormatter {
public:
    virtual std::string format(const ProjectMetadata& metadata) = 0;
    virtual std::string getFormatName() const = 0;
    virtual std::string getFileExtension() const = 0;
};

class MarkdownFormatter : public OutputFormatter { /* ... */ };
class XMLFormatter : public OutputFormatter { /* ... */ };
class YAMLFormatter : public OutputFormatter { /* ... */ };
```

## 🧪 **테스트 아키텍처**

### **테스트 구조**
```
Tests/
├── test_main.cpp              # 테스트 프레임워크
├── test_treesitter_engine.cpp # 파싱 테스트
├── test_unity_analyzer.cpp    # Unity 분석 테스트
├── test_metadata_generator.cpp # 메타데이터 테스트
├── test_core_engine.cpp       # 통합 테스트
└── SampleUnityProject/        # 테스트용 Unity 스크립트
    ├── PlayerController.cs
    ├── GameManager.cs
    ├── EnemyAI.cs
    └── UIController.cs
```

### **테스트 커버리지**
- **파싱 정확도**: 다양한 C# 구문 테스트
- **Unity 메서드 감지**: 25개 생명주기 메서드
- **패턴 인식**: 10개 디자인 패턴
- **의존성 분석**: 복잡한 컴포넌트 관계
- **성능 테스트**: 대용량 파일 처리

## 🔧 **빌드 시스템**

### **CMake 구조**
```cmake
# 루트 CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(UnityLLMContextGenerator)

# 컴파일러 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 외부 의존성
find_package(PkgConfig REQUIRED)
find_library(TREE_SITTER_LIB tree-sitter)
find_package(nlohmann_json REQUIRED)

# 모듈별 빌드
add_subdirectory(Core)      # 핵심 라이브러리
add_subdirectory(CLI)       # CLI 실행파일
add_subdirectory(Tests)     # 테스트
```

### **의존성 관리**
- **Tree-sitter**: C# 구문 분석
- **nlohmann/json**: JSON 처리
- **CMake**: 빌드 시스템
- **C++17**: 현대적 C++ 기능 활용

## 🚀 **배포 아키텍처 (계획)**

### **Unity Package 구조**
```
com.unity.llm-context-generator/
├── Runtime/
│   ├── Scripts/           # C# 런타임 스크립트
│   └── Plugins/           # Native DLL
├── Editor/
│   ├── Scripts/           # Editor 스크립트
│   └── Windows/           # Editor 윈도우
├── Documentation~/        # 문서
└── package.json          # Package 정의
```

### **CLI 배포**
- **Windows**: .exe + DLL 번들
- **macOS**: .app 또는 Homebrew
- **Linux**: .deb/.rpm 패키지 또는 AppImage

## 📊 **메트릭 및 모니터링**

### **성능 메트릭**
- 파싱 속도 (라인/초)
- 메모리 사용량 (MB)
- 분석 정확도 (%)
- 캐시 적중률 (%)

### **품질 메트릭**
- 코드 커버리지
- 순환 복잡도
- 의존성 그래프 복잡성
- 패턴 감지 신뢰도

---

**🎯 이 아키텍처는 확장 가능하고 성능 최적화된 Unity-LLM 브리지를 제공합니다.**