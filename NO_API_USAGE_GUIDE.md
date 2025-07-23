# 🆓 API 키 없이 Unity Smart Context 사용하기

Claude Pro Plan만으로도 Unity Smart Context Generator를 효과적으로 활용할 수 있는 다양한 방법들을 소개합니다.

## 💡 **핵심 아이디어**

Claude API는 유료이지만, **Claude Pro Plan ($20/월)**을 이미 사용 중이라면 다음 방법들로 API 비용 없이 동일한 효과를 얻을 수 있습니다:

1. **MCP (Model Context Protocol)** - Claude Desktop 직접 연동
2. **웹 인터페이스** - 브라우저에서 복사/붙여넣기
3. **VS Code 확장** - 개발 환경 통합
4. **CLI + 수동 복사** - 터미널에서 결과 복사

---

## 🔌 **방법 1: MCP 서버 (추천)**

Claude Desktop에서 직접 Unity 분석을 실행할 수 있습니다.

### 설치 및 설정

```bash
# Python 의존성 설치
cd MCP
pip install -r requirements.txt

# Claude Desktop 설정 파일 편집
# macOS: ~/Library/Application Support/Claude/claude_desktop_config.json
# Windows: %APPDATA%/Claude/claude_desktop_config.json
```

**claude_desktop_config.json**:
```json
{
  "mcpServers": {
    "unity-context-generator": {
      "command": "python",
      "args": ["/path/to/UnitySmartContext/MCP/mcp_server.py"],
      "cwd": "/path/to/UnitySmartContext"
    }
  }
}
```

### 사용법

Claude Desktop에서 다음과 같이 사용:

```
Unity 프로젝트를 분석해줘:
/path/to/Assets/Scripts

단일 파일 분석:
PlayerController.cs 파일을 분석해줘

Unity API 패턴 확인:
지원되는 Unity API 패턴들을 보여줘
```

### 장점
- ✅ Claude Desktop에서 직접 실행
- ✅ API 비용 없음 (Pro Plan만 필요)
- ✅ 실시간 대화형 분석
- ✅ 자동화된 워크플로우

---

## 🌐 **방법 2: 웹 인터페이스**

브라우저에서 Unity 코드를 분석하고 Claude용 컨텍스트를 생성합니다.

### 실행

```bash
cd WebInterface
pip install flask
python server.py
```

브라우저에서 `http://localhost:5000` 접속

### 특징

- **코드 입력**: 직접 코드 붙여넣기
- **파일 업로드**: 로컬 파일 분석
- **프로젝트 분석**: 전체 디렉토리 스캔
- **Claude 컨텍스트**: 원클릭 복사

### 워크플로우

1. **웹 인터페이스에서 Unity 코드 분석**
2. **"Claude 컨텍스트" 탭에서 결과 복사**
3. **Claude.ai에서 붙여넣기**
4. **"이 코드를 어떻게 개선할 수 있나요?" 질문**

### 장점
- ✅ 설치 없이 바로 사용
- ✅ 시각적 인터페이스
- ✅ 결과 다운로드 가능
- ✅ 여러 분석 형식 지원

---

## 🔧 **방법 3: VS Code 확장**

개발 환경에서 직접 Unity 분석을 실행하고 Claude용 컨텍스트를 생성합니다.

### 설치

```bash
cd VSCodeExtension
npm install
npm run compile
```

VS Code에서 F5를 눌러 확장 개발 모드 실행

### 사용법

1. **Unity C# 파일 우클릭** → "Unity: Analyze Current File for Claude"
2. **폴더 우클릭** → "Unity: Analyze Workspace for Claude"
3. **명령 팔레트** (Ctrl+Shift+P) → "Unity: Show Supported API Patterns"

### 특징

- **실시간 분석**: 파일 저장 시 자동 분석 (옵션)
- **컨텍스트 생성**: Claude용 최적화된 분석 결과
- **클립보드 복사**: 원클릭으로 Claude에 붙여넣기
- **개발 통합**: VS Code 워크플로우에 완전 통합

### 장점
- ✅ 개발 환경 통합
- ✅ 실시간 분석
- ✅ 자동화 가능
- ✅ 프로젝트 컨텍스트 유지

---

## 📋 **방법 4: CLI + 수동 복사**

가장 간단한 방법으로, CLI 도구로 분석하고 결과를 수동으로 복사합니다.

### 기본 사용법

```bash
# 단일 파일 분석
./unity_context_generator PlayerController.cs

# 프로젝트 분석
./unity_context_generator --directory Assets/Scripts

# LLM 최적화된 형식
./unity_context_generator --format llm Assets/Scripts
```

### Claude용 컨텍스트 생성

```bash
# 분석 실행
./unity_context_generator PlayerController.cs

# Claude용 프롬프트 복사
cat unity_context_analysis/llm_prompt.md
```

### 워크플로우

1. **CLI로 Unity 코드 분석**
2. **`llm_prompt.md` 파일 내용 복사**
3. **Claude.ai에 붙여넣기**
4. **추가 질문 및 개선 요청**

### 장점
- ✅ 설정 불필요
- ✅ 스크립트 자동화 가능
- ✅ 배치 처리 지원
- ✅ CI/CD 통합 가능

---

## 🎯 **실제 사용 예시**

### 시나리오: PlayerController.cs 개선

**1단계**: Unity 코드 분석
```bash
./unity_context_generator PlayerController.cs
```

**2단계**: Claude용 컨텍스트 복사
```bash
cat unity_context_analysis/llm_prompt.md | pbcopy  # macOS
cat unity_context_analysis/llm_prompt.md | clip    # Windows
```

**3단계**: Claude.ai에서 대화
```
[붙여넣기한 컨텍스트]

이 PlayerController 스크립트를 성능과 베스트 프랙티스 관점에서 개선해주세요. 
특히 GetComponent 호출과 Update 루프 최적화를 중심으로 리팩토링해주세요.
```

**결과**: Claude가 Unity 컨텍스트를 완전히 이해하고 구체적인 개선 코드를 제공

---

## 📊 **방법별 비교**

| 방법 | 설정 복잡도 | 사용 편의성 | 자동화 | 실시간성 |
|------|-------------|-------------|--------|-----------|
| **MCP 서버** | 중간 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **웹 인터페이스** | 낮음 | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐ |
| **VS Code 확장** | 중간 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| **CLI + 수동** | 없음 | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ |

## 🎉 **추천 조합**

### 개인 개발자
- **MCP 서버** (실시간 대화형 분석)
- **VS Code 확장** (개발 중 즉시 분석)

### 팀 개발
- **웹 인터페이스** (팀원들이 쉽게 접근)
- **CLI + 자동화** (CI/CD 통합)

### 학습 목적
- **CLI + 수동 복사** (Unity 분석 과정 이해)
- **웹 인터페이스** (시각적 결과 확인)

---

## 💰 **비용 절약 효과**

### Claude API vs Pro Plan 비교

**Claude API 사용 시**:
- 분석 1회당 ~$0.02 (약 25원)
- 월 100회 분석 = ~$2 (약 2,500원)
- Pro Plan + API = $20 + $2 = $22/월

**Pro Plan만 사용 시**:
- 분석 횟수 제한 없음
- 추가 비용 없음
- 총 비용 = $20/월

**절약액**: 월 $2 이상 + 사용량에 따라 더 많은 절약 가능

---

## 🚀 **다음 단계**

1. **원하는 방법 선택** (MCP 서버 추천)
2. **설치 및 설정** (각 방법별 가이드 참조)
3. **Unity 프로젝트로 테스트**
4. **Claude와 함께 코드 개선**

API 키 없이도 Unity Smart Context Generator의 모든 기능을 활용하여 AI 기반 Unity 개발의 이점을 누리세요! 🎮✨