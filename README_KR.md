# Unity Smart Context Generator

🎮 **AI-Powered Unity Code Analysis and Context Generation Tool**

Unity Smart Context Generator는 Unity 프로젝트의 C# 스크립트를 분석하여 AI 기반의 지능형 코드 리뷰, 아키텍처 분석, 그리고 LLM을 위한 최적화된 컨텍스트를 생성하는 도구입니다.

![Unity](https://img.shields.io/badge/Unity-2019.4+-black?style=flat-square&logo=unity)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=cplusplus)
![Claude](https://img.shields.io/badge/Claude-3.5_Sonnet-orange?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

## ✨ 주요 기능

### 🔍 **지능형 코드 분석**
- **Unity API 탐지**: 30+ Unity API 패턴 자동 인식 및 분류
- **성능 분석**: 성능에 민감한 API 호출 식별
- **아키텍처 분석**: 컴포넌트 의존성 및 설계 패턴 분석
- **AI 기반 코드 리뷰**: Claude AI를 통한 전문적 코드 리뷰

### 🎯 **Unity 전문 분석**
- **MonoBehaviour 분석**: Unity 라이프사이클 메서드 패턴 분석
- **ScriptableObject 탐지**: 데이터 에셋 구조 분석
- **컴포넌트 의존성**: GameObject-Component 관계 매핑
- **디자인 패턴 탐지**: Singleton, Observer 등 Unity 패턴 인식

### 🤖 **AI 통합**
- **Claude API 통합**: Anthropic Claude 3.5 Sonnet 지원
- **컨텍스트 인식 분석**: Unity 프로젝트 맥락을 이해하는 AI 분석
- **성능 최적화 제안**: AI 기반 성능 개선 권장사항
- **베스트 프랙티스**: Unity 개발 모범 사례 제안

### 🛠️ **다양한 사용 방식**
- **CLI 도구**: 명령줄에서 배치 분석
- **Unity 에디터 플러그인**: 실시간 분석 및 GUI
- **CI/CD 통합**: 자동화된 코드 리뷰 파이프라인

## 🚀 빠른 시작

### 필수 요구사항
- C++17 지원 컴파일러 (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.15+
- libcurl4-openssl-dev
- nlohmann/json

### 설치

```bash
# 저장소 클론
git clone https://github.com/your-username/UnitySmartContext.git
cd UnitySmartContext

# 의존성 설치 (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev

# 빌드
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 기본 사용법

```bash
# 단일 파일 분석
./unity_context_generator PlayerController.cs

# 프로젝트 디렉토리 분석
./unity_context_generator --directory Assets/Scripts

# AI 분석 활성화 (API 키 필요)
export ANTHROPIC_API_KEY="your-api-key"
./unity_context_generator --directory Assets/Scripts --verbose

# Unity 에디터 플러그인 사용
# Unity 에디터 → Window → LLM Context Generator
```

## 📊 분석 결과 예시

### 생성되는 파일들
```
unity_context_analysis/
├── project_metadata.json      # 상세 컴포넌트 메타데이터
├── project_context.json       # 아키텍처 인사이트
├── llm_optimized.json         # LLM 친화적 컨텍스트
├── summary.md                 # 프로젝트 요약
├── detailed_report.md         # 상세 분석 리포트
└── llm_prompt.md             # LLM용 컨텍스트 프롬프트
```

### Unity API 탐지 결과
```
[UnityAPIDetector] Detected 5 Unity API calls in PlayerController.cs
✅ Analysis Duration: 264ms
🎮 Project Type: Generic Game
🏗️ Architecture: Component-based
📈 Quality Score: 87%

📦 Component Summary:
   • MonoBehaviours: 3
   • System Groups: 2
   • Dependencies: 5

🎯 AI 분석 결과:
   • Performance Issues: GetComponent 캐싱 권장
   • Best Practices: FixedUpdate에서 물리 처리 권장
   • Architecture: Observer 패턴 적용 고려
```

## 🎮 Unity 에디터 플러그인

### 설치
1. `UnityPlugin/` 폴더를 Unity 프로젝트의 `Assets/` 폴더에 복사
2. Unity 에디터에서 `Window → LLM Context Generator` 열기

### 기능
- **실시간 분석**: 파일 변경 시 자동 분석
- **AI 인사이트**: Claude AI 기반 코드 제안
- **컴포넌트 시각화**: 의존성 그래프 표시
- **성능 모니터링**: 성능 이슈 실시간 알림

## 🔧 고급 설정

### AI 분석 설정
```bash
# 환경 변수로 API 키 설정
export ANTHROPIC_API_KEY="your-anthropic-api-key"

# 또는 설정 파일 생성
echo '{
  "ai": {
    "enable_ai_analysis": true,
    "api_key": "your-api-key",
    "model_name": "claude-3-5-sonnet-20241022"
  }
}' > unity_context_config.json
```

### CLI 옵션
```bash
# 자세한 옵션 보기
./unity_context_generator --help

# 특정 분석만 수행
./unity_context_generator --no-dependencies --no-patterns Assets/Scripts/

# 출력 형식 지정
./unity_context_generator --format llm --output ./analysis/ PlayerController.cs

# 실시간 분석 (파일 변경 감지)
./unity_context_generator --watch Assets/Scripts/
```

## 🏗️ 아키텍처

```
┌─────────────────────────────────────────────────────────────────┐
│                        CoreEngine                               │
├─────────────────────────────────────────────────────────────────┤
│  ├── TreeSitter Parser (C# AST 분석)                            │
│  ├── Unity Analyzer (컴포넌트 분석)                             │
│  ├── AI Analysis Pipeline (Claude API 통합)                     │
│  │   ├── UnityAPIDetector                                      │
│  │   ├── ClaudeAnalyzer                                        │
│  │   └── HTTPClient                                            │
│  └── Metadata Generator (JSON/Markdown 출력)                   │
└─────────────────────────────────────────────────────────────────┘
```

### 핵심 구성요소
- **TreeSitter Engine**: C# 소스코드 파싱
- **Unity Analyzer**: Unity 특화 분석 (MonoBehaviour, ScriptableObject)
- **AI Integration**: Claude API를 통한 지능형 분석
- **Metadata Generator**: 다양한 형식의 결과 출력
- **Unity Plugin**: 에디터 통합 및 실시간 분석

## 📚 문서

- [**Claude AI 통합 가이드**](CLAUDE_AI_INTEGRATION.md) - AI 분석 시스템 상세 설명
- [**개발 현황**](DEVELOPMENT_STATUS.md) - 현재 개발 진행 상황
- [**아키텍처 문서**](ARCHITECTURE.md) - 시스템 설계 및 구조
- [**다음 단계**](NEXT_STEPS.md) - 향후 개발 계획

## 🤝 기여하기

Unity Smart Context Generator는 오픈소스 프로젝트입니다! 기여를 환영합니다.

### 기여 방법
1. 이 저장소를 Fork
2. Feature 브랜치 생성 (`git checkout -b feature/amazing-feature`)
3. 변경사항 커밋 (`git commit -m 'Add amazing feature'`)
4. 브랜치에 Push (`git push origin feature/amazing-feature`)
5. Pull Request 생성

### 버그 리포트 및 기능 제안
- [Issues](https://github.com/your-username/UnitySmartContext/issues)를 통해 버그 리포트 또는 기능 제안
- Unity 버전, OS, 오류 메시지 등 상세 정보 포함

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 있습니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 🙏 감사의 말

- [Tree-sitter](https://tree-sitter.github.io/) - C# 파싱 엔진
- [nlohmann/json](https://github.com/nlohmann/json) - JSON 처리
- [Anthropic Claude](https://www.anthropic.com/) - AI 분석 엔진
- Unity Technologies - Unity 엔진 및 생태계

## 📞 연락처

- 개발자: [GitHub Profile](https://github.com/your-username)
- 이슈 및 질문: [GitHub Issues](https://github.com/your-username/UnitySmartContext/issues)

---

⭐ **이 프로젝트가 도움이 되었다면 Star를 눌러주세요!**