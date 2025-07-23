# Unity LLM Context Generator - 다음 단계 가이드

## 🎯 즉시 실행 가능한 작업들

### 🔧 **개발 환경 확인**

1. **현재 구현 상태 검증**
```bash
cd /mnt/c/Users/tjdql/Desktop/UnitySmartContext

# 프로젝트 구조 확인
ls -la
tree -L 2

# 빌드 시스템 테스트 (Tree-sitter 설치 필요)
mkdir -p build
cd build
cmake ..
```

2. **의존성 설치 (필요시)**
```bash
# Ubuntu/Debian
sudo apt-get install libtree-sitter-dev nlohmann-json3-dev

# macOS (Homebrew)
brew install tree-sitter nlohmann-json

# Windows (vcpkg)
vcpkg install tree-sitter nlohmann-json
```

### 📋 **Phase 2 작업 계획 (Unity Editor 통합)**

## 🚀 **Week 5-6: Unity Editor Plugin 핵심 개발**

### **Priority 1: C# Unity Plugin 기본 구조**

1. **Unity Package 구조 생성**
```bash
mkdir -p UnityPlugin/Runtime/Scripts
mkdir -p UnityPlugin/Editor/Scripts  
mkdir -p UnityPlugin/Editor/Windows
mkdir -p UnityPlugin/Native/x64
```

2. **필요한 Unity C# 스크립트들**

**UnityPlugin/Runtime/Scripts/ContextAnalyzer.cs**
```csharp
using UnityEngine;
using System.Runtime.InteropServices;

namespace UnityLLMContext 
{
    public class ContextAnalyzer : MonoBehaviour
    {
        [DllImport("unity_context_core")]
        private static extern IntPtr AnalyzeProject(string projectPath);
        
        [DllImport("unity_context_core")]
        private static extern void FreeResults(IntPtr results);
        
        public static string AnalyzeCurrentProject()
        {
            string projectPath = Application.dataPath;
            IntPtr result = AnalyzeProject(projectPath);
            // JSON 결과 처리
            return ProcessResults(result);
        }
    }
}
```

**UnityPlugin/Editor/Scripts/ContextAnalyzerWindow.cs**
```csharp
using UnityEngine;
using UnityEditor;

namespace UnityLLMContext.Editor
{
    public class ContextAnalyzerWindow : EditorWindow
    {
        [MenuItem("Tools/LLM Context Generator")]
        public static void ShowWindow()
        {
            GetWindow<ContextAnalyzerWindow>("LLM Context");
        }
        
        void OnGUI()
        {
            GUILayout.Label("Unity LLM Context Generator", EditorStyles.boldLabel);
            
            if (GUILayout.Button("Analyze Project"))
            {
                AnalyzeProject();
            }
            
            if (GUILayout.Button("Generate LLM Prompt"))
            {
                GeneratePrompt();
            }
        }
        
        private void AnalyzeProject()
        {
            string result = ContextAnalyzer.AnalyzeCurrentProject();
            Debug.Log($"Analysis complete: {result}");
        }
    }
}
```

### **Priority 2: C++ DLL 인터페이스 구현**

3. **C++ 내보내기 인터페이스 추가**

**UnityPlugin/Native/UnityInterface.h**
```cpp
#pragma once

#ifdef _WIN32
#define UNITY_EXPORT __declspec(dllexport)
#else
#define UNITY_EXPORT
#endif

extern "C" {
    UNITY_EXPORT const char* AnalyzeProject(const char* projectPath);
    UNITY_EXPORT const char* AnalyzeFile(const char* filePath);
    UNITY_EXPORT const char* GenerateLLMPrompt(const char* analysisResult);
    UNITY_EXPORT void FreeString(const char* str);
}
```

**UnityPlugin/Native/UnityInterface.cpp**
```cpp
#include "UnityInterface.h"
#include "../../Core/CoreEngine.h"
#include <cstring>

using namespace UnityContextGen;

extern "C" {
    
UNITY_EXPORT const char* AnalyzeProject(const char* projectPath) {
    try {
        CoreEngine engine;
        AnalysisOptions options;
        options.export_json = false;
        
        auto result = engine.analyzeDirectory(projectPath, options);
        
        if (result.success) {
            // JSON 문자열로 변환
            auto json_result = /* JSON 변환 로직 */;
            return strdup(json_result.c_str());
        }
        return strdup("{\"error\":\"Analysis failed\"}");
        
    } catch (const std::exception& e) {
        return strdup("{\"error\":\"Exception occurred\"}");
    }
}

UNITY_EXPORT void FreeString(const char* str) {
    free(const_cast<char*>(str));
}

}
```

### **Priority 3: Unity Package 정의**

4. **package.json 작성**
```json
{
  "name": "com.unity.llm-context-generator",
  "version": "1.0.0",
  "displayName": "LLM Context Generator",
  "description": "Generates LLM-friendly context metadata for Unity C# projects",
  "unity": "2022.3",
  "dependencies": {},
  "keywords": [
    "llm",
    "ai",
    "context",
    "analysis",
    "claude-code"
  ],
  "author": {
    "name": "Unity LLM Context Team"
  }
}
```

## 🗓️ **Week 7: DLL 통합 및 테스트**

### **Priority 1: 빌드 시스템 확장**

5. **CMake Unity 통합 추가**
```cmake
# UnityPlugin/Native/CMakeLists.txt
add_library(unity_context_interface SHARED
    UnityInterface.cpp
    UnityInterface.h
)

target_link_libraries(unity_context_interface
    unity_context_core
    ${TREE_SITTER_LIB}
    nlohmann_json::nlohmann_json
)

# Unity에서 요구하는 DLL 이름
set_target_properties(unity_context_interface PROPERTIES
    OUTPUT_NAME "unity_context_core"
)
```

### **Priority 2: 에러 처리 및 안정성**

6. **예외 처리 강화**
```cpp
// 안전한 문자열 변환 함수
std::string SafeJsonToString(const nlohmann::json& json) {
    try {
        return json.dump(2);
    } catch (const std::exception& e) {
        return "{\"error\":\"JSON serialization failed\"}";
    }
}

// Unity 콜백을 위한 안전한 래퍼
extern "C" UNITY_EXPORT const char* SafeAnalyzeProject(const char* projectPath) {
    if (!projectPath) {
        return strdup("{\"error\":\"Null project path\"}");
    }
    
    // 실제 분석 로직
    return AnalyzeProject(projectPath);
}
```

### **Priority 3: Unity Editor 통합 테스트**

7. **Unity 테스트 프로젝트 생성**
```bash
# Unity 2022.3 LTS로 새 프로젝트 생성
# Package Manager에서 로컬 패키지 추가
# Editor Window 테스트
# Runtime 스크립트 테스트
```

## 📅 **Week 8: 완성 및 문서화**

### **Priority 1: 사용자 경험 개선**

8. **진행률 표시 및 비동기 처리**
```csharp
public class ContextAnalyzerWindow : EditorWindow
{
    private bool isAnalyzing = false;
    private float progress = 0f;
    
    void OnGUI()
    {
        if (isAnalyzing)
        {
            EditorGUI.ProgressBar(new Rect(10, 30, 300, 20), progress, "Analyzing...");
            return;
        }
        
        if (GUILayout.Button("Analyze Project"))
        {
            StartAsyncAnalysis();
        }
    }
    
    async void StartAsyncAnalysis()
    {
        isAnalyzing = true;
        progress = 0f;
        
        await Task.Run(() => {
            // 백그라운드 분석
            string result = ContextAnalyzer.AnalyzeCurrentProject();
            // 결과 처리
        });
        
        isAnalyzing = false;
        Repaint();
    }
}
```

### **Priority 2: 문서화 완성**

9. **사용자 가이드 작성**

**UnityPlugin/Documentation~/UnityLLMContextGenerator.md**
```markdown
# Unity LLM Context Generator 사용 가이드

## 설치 방법
1. Package Manager 열기
2. "Add package from disk" 선택  
3. package.json 파일 선택
4. Import 완료

## 사용 방법
1. Tools > LLM Context Generator 메뉴 선택
2. "Analyze Project" 버튼 클릭
3. 생성된 컨텍스트를 Claude Code에 복사

## 출력 파일
- `Assets/LLMContext/project_metadata.json`
- `Assets/LLMContext/llm_prompt.md`
- `Assets/LLMContext/component_analysis.json`
```

### **Priority 3: 배포 준비**

10. **Unity Package 빌드 스크립트**
```bash
#!/bin/bash
# build_unity_package.sh

# C++ DLL 빌드
cd build
make unity_context_interface

# DLL을 Unity Plugin 폴더로 복사
cp libunity_context_core.so ../UnityPlugin/Native/x64/
# Windows: copy unity_context_core.dll ../UnityPlugin/Native/x64/

# Unity Package 생성
cd ..
tar -czf unity-llm-context-generator.tgz UnityPlugin/

echo "Unity Package created: unity-llm-context-generator.tgz"
```

## 🔮 **Phase 3 준비 작업**

### **Claude Code 통합 계획**

11. **Claude Code 확장 개발 준비**
```typescript
// claude-code-unity-extension/src/extension.ts
import * as vscode from 'vscode';

export function activate(context: vscode.ExtensionContext) {
    const command = vscode.commands.registerCommand('unity-llm.analyzeProject', () => {
        // Unity 프로젝트 감지
        // 분석 도구 실행
        // 결과를 Claude Code 컨텍스트로 주입
    });
    
    context.subscriptions.push(command);
}
```

### **표준화 작업**

12. **메타데이터 스키마 정의**
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Unity LLM Context Metadata",
  "type": "object",
  "properties": {
    "project_context": {
      "type": "object",
      "properties": {
        "type": {"type": "string"},
        "architecture": {"type": "string"},
        "complexity": {"type": "string"}
      }
    },
    "components": {
      "type": "object",
      "patternProperties": {
        "^[A-Za-z][A-Za-z0-9]*$": {
          "$ref": "#/definitions/component"
        }
      }
    }
  },
  "definitions": {
    "component": {
      "type": "object",
      "properties": {
        "purpose": {"type": "string"},
        "dependencies": {"type": "array"},
        "lifecycle_usage": {"type": "array"}
      }
    }
  }
}
```

## ⚠️ **주의사항 및 고려사항**

### **기술적 고려사항**
1. **Unity 버전 호환성**: 2022.3 LTS부터 지원
2. **플랫폼 지원**: Windows, macOS, Linux
3. **성능**: 대용량 프로젝트 (1000+ 스크립트) 지원
4. **메모리**: Unity Editor 내에서 안정적 동작

### **사용자 경험 고려사항**
1. **직관적 UI**: 최소한의 설정으로 바로 사용 가능
2. **빠른 피드백**: 진행률 표시 및 에러 메시지
3. **결과 활용**: 생성된 컨텍스트의 쉬운 복사/붙여넣기
4. **문서화**: 충분한 예제와 가이드

## 📞 **팀 협업 가이드**

### **개발자 역할 분담**
- **Backend 개발자**: C++ DLL 인터페이스 완성
- **Unity 개발자**: C# Plugin 및 Editor 통합
- **DevOps**: 빌드 시스템 및 배포 자동화
- **QA**: 다양한 Unity 프로젝트에서 테스트

### **일일 체크인**
1. **진행상황 공유**: 매일 오전 스탠드업
2. **블로커 해결**: 즉시 에스컬레이션
3. **코드 리뷰**: PR 당일 리뷰 완료
4. **통합 테스트**: 매일 저녁 자동 실행

---

**🎯 Phase 2 성공적 완료를 위한 체계적 접근법**  
**🚀 실용적이고 안정적인 Unity Plugin 구축이 목표**