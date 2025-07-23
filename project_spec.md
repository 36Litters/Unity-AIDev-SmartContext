# Unity LLM Context Generator 프로젝트 명세서

## 📋 프로젝트 개요

### 목표
Unity C# 개발에서 LLM(Claude Code 등)이 코드의 맥락을 더 효율적으로 파악할 수 있도록 하는 메타데이터 자동 생성 도구 개발

### 핵심 문제 정의
- **컨텍스트 길이 제한**: LLM이 긴 코드의 전체 맥락을 파악하기 어려움
- **Unity 특수성**: MonoBehaviour 생명주기, 컴포넌트 관계, Scene 구조 등 Unity 특화 패턴 이해 부족
- **토큰 효율성**: 프로젝트 전체를 이해하기 위해 과도한 토큰 사용
- **코드 일관성**: LLM이 기존 프로젝트 패턴을 파악하고 일관된 코드 생성의 어려움

## 🎯 해결 방안

### LLM 친화적 코드 구조 정의
```csharp
// 기존 코드의 문제점
public class PlayerController : MonoBehaviour 
{
    // 책임이 명확하지 않고, 맥락 파악이 어려운 구조
    void Update() { /* 입력, 점프, 이동이 섞여있음 */ }
}

// 개선된 구조 (목표)
[ComponentPurpose("Handles player movement, jumping, and ground detection")]
[Dependencies("Rigidbody", "Input System")]
public class PlayerController : MonoBehaviour 
{
    // === INPUT HANDLING BLOCK ===
    [Purpose("Process player input and trigger actions")]
    void Update() { /* 명확한 책임 분리 */ }
    
    // === PHYSICS BLOCK ===
    [Purpose("Apply movement forces to player")]
    void FixedUpdate() { /* 물리 처리만 */ }
}
```

### 자동 생성 메타데이터
- **컴포넌트 의존성 그래프**: 어떤 컴포넌트들이 상호작용하는지
- **Unity 생명주기 매핑**: Awake → Start → Update → FixedUpdate 플로우
- **데이터 플로우 추적**: Input → State → Physics → Rendering 흐름
- **프로젝트 아키텍처 요약**: 전체 구조를 한눈에 파악 가능한 요약

## 🛠 기술 아키텍처

### 선택된 기술 스택
- **Core Engine**: C++ + Tree-sitter (성능 최적화)
- **Unity Integration**: C# Plugin (Unity Editor 연동)
- **CLI Tool**: C++ 실행파일 (외부 도구 지원)
- **Build System**: CMake (크로스 플랫폼 지원)

### Tree-sitter 선택 이유
- GitHub Copilot과 동급의 파싱 성능
- 실시간 구문 분석 지원
- C# 언어 지원 내장
- 메모리 효율적인 AST 생성

### 프로젝트 구조
```
UnityLLMContextGenerator/
├── Core/                          # C++ 엔진
│   ├── TreeSitterEngine/         # Tree-sitter 래퍼
│   ├── UnityAnalyzer/            # Unity 특화 분석
│   ├── MetadataGenerator/        # 메타데이터 생성
│   └── ContextSummarizer/        # 프로젝트 요약
├── UnityPlugin/                  # Unity Editor 연동
│   ├── Editor/                   # Unity Editor 스크립트
│   ├── Runtime/                  # 런타임 컴포넌트
│   └── Native/                   # C++ DLL 래퍼
├── CLI/                          # 커맨드라인 도구
│   ├── BatchAnalyzer/           # 배치 분석
│   └── ClaudeCodeIntegration/   # Claude Code 연동
└── Tests/                        # 테스트 프로젝트
    └── SampleUnityProject/       # 검증용 Unity 프로젝트
```

## 🎮 Unity 특화 기능

### MonoBehaviour 생명주기 분석
- Awake, Start, Update, FixedUpdate, LateUpdate 실행 순서 추적
- 각 메서드의 역할과 의존성 자동 추출

### 컴포넌트 관계 매핑
- GetComponent 호출 패턴 분석
- 컴포넌트 간 데이터 플로우 추적
- Required 컴포넌트 의존성 자동 감지

### Scene 구조 이해
- Prefab과 Script 연결 관계
- GameObject 계층 구조 분석
- Unity Event 플로우 추적

## 📊 출력 예시

### 생성될 메타데이터 포맷
```json
{
  "project_summary": {
    "architecture_pattern": "Component-based game architecture",
    "key_systems": ["Player Control", "Enemy AI", "Physics Interaction"],
    "data_flow": "Input → Game Logic → Physics → Rendering"
  },
  "components": {
    "PlayerController": {
      "purpose": "Handles player movement, jumping, and ground detection",
      "dependencies": ["Rigidbody", "Input System"],
      "lifecycle_usage": ["Update", "FixedUpdate", "OnCollisionEnter"],
      "responsibility_blocks": {
        "input_handling": "Process WASD and jump input",
        "physics_update": "Apply movement forces",
        "collision_detection": "Handle ground contact"
      }
    }
  }
}
```

### LLM 프롬프트 최적화
```
// 기존 방식 (토큰 과다 사용)
"다음 5000줄 코드를 분석해서 PlayerController에 대시 기능을 추가해줘..."

// 개선된 방식 (토큰 효율적)
"Project: 3D 플랫포머 게임
Architecture: Component-based 
PlayerController: 움직임/점프/지면감지 담당 (Rigidbody + Input 의존)
Request: 기존 패턴을 유지하며 대시 기능 추가"
```

## 🎯 성공 지표

### 성능 목표
- **분석 속도**: 1000줄 프로젝트 < 1초 분석
- **메모리 사용량**: 대형 프로젝트도 512MB 이하
- **정확도**: Unity 컴포넌트 관계 90% 이상 정확 추출

### 사용성 목표
- **토큰 효율성**: 기존 대비 80% 토큰 절약
- **코드 일관성**: LLM 생성 코드의 프로젝트 패턴 준수율 95%
- **개발 생산성**: 새 기능 추가 시간 50% 단축

## 🚀 개발 단계

### Phase 1: Core Engine (4주)
- Tree-sitter C# 파서 통합
- 기본 AST 분석 기능
- Unity MonoBehaviour 감지

### Phase 2: Unity Integration (3주)
- Unity Editor Plugin 개발
- 실시간 메타데이터 표시
- DLL 통합 및 테스트

### Phase 3: Claude Code Integration (2주)
- CLI 도구 완성
- 표준 메타데이터 포맷 정의
- Claude Code 명령어 연동

### Phase 4: 최적화 및 배포 (3주)
- 성능 최적화
- 문서화 및 예제
- 오픈소스 배포 준비

## 🔧 기술적 도전과제

### Tree-sitter 통합
- C# grammar의 Unity 특화 확장
- 복잡한 제네릭 타입 파싱
- 어트리뷰트와 메타데이터 추출

### Unity 특수성 처리
- 직렬화된 필드 분석
- Inspector 값과 코드 연결
- Scene 파일 파싱

### 성능 최적화
- 대형 프로젝트 처리
- 실시간 분석 vs 배치 처리
- 메모리 효율적 AST 순회

## 📈 확장 가능성

### 다른 게임 엔진 지원
- Unreal Engine C++ 지원
- Godot GDScript 지원

### IDE 통합
- Visual Studio 확장
- Rider 플러그인
- VSCode 확장

### AI 도구 연동
- GitHub Copilot 호환
- 다른 LLM 도구들과 연동

---

*이 명세서는 Claude Code와 같은 AI 도구가 프로젝트의 전체 맥락을 이해하고 효율적으로 개발할 수 있도록 작성되었습니다.*