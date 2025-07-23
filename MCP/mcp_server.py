#!/usr/bin/env python3
"""
Unity Smart Context MCP Server
Claude Desktop에서 직접 사용할 수 있는 MCP 서버 구현
"""

import asyncio
import json
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

from mcp.server.models import InitializationOptions
from mcp.server.server import NotificationOptions, Server
from mcp.server.stdio import stdio_server
from mcp.types import (
    CallToolRequest,
    CallToolResult,
    ListToolsRequest,
    ListToolsResult,
    TextContent,
    Tool,
)

# Unity Context Generator 실행 파일 경로
UNITY_CONTEXT_GENERATOR = "./build/CLI/unity_context_generator"

class UnityContextMCPServer:
    def __init__(self):
        self.server = Server("unity-context-generator")
        self._setup_handlers()
    
    def _setup_handlers(self):
        @self.server.list_tools()
        async def handle_list_tools() -> ListToolsResult:
            """사용 가능한 도구들을 나열합니다."""
            return ListToolsResult(
                tools=[
                    Tool(
                        name="analyze_unity_file",
                        description="단일 Unity C# 스크립트를 분석합니다",
                        inputSchema={
                            "type": "object",
                            "properties": {
                                "file_path": {
                                    "type": "string",
                                    "description": "분석할 Unity C# 파일 경로"
                                },
                                "analysis_type": {
                                    "type": "string",
                                    "enum": ["basic", "detailed", "llm_optimized"],
                                    "description": "분석 유형",
                                    "default": "detailed"
                                }
                            },
                            "required": ["file_path"]
                        }
                    ),
                    Tool(
                        name="analyze_unity_project",
                        description="Unity 프로젝트 디렉토리 전체를 분석합니다",
                        inputSchema={
                            "type": "object",
                            "properties": {
                                "directory_path": {
                                    "type": "string",
                                    "description": "분석할 Unity 프로젝트 디렉토리 경로"
                                },
                                "include_ai_analysis": {
                                    "type": "boolean",
                                    "description": "AI 분석 포함 여부 (API 키 필요)",
                                    "default": False
                                }
                            },
                            "required": ["directory_path"]
                        }
                    ),
                    Tool(
                        name="get_unity_api_patterns",
                        description="지원되는 Unity API 패턴 목록을 가져옵니다",
                        inputSchema={
                            "type": "object",
                            "properties": {}
                        }
                    ),
                    Tool(
                        name="generate_llm_context",
                        description="분석 결과를 LLM 친화적 컨텍스트로 변환합니다",
                        inputSchema={
                            "type": "object",
                            "properties": {
                                "analysis_result_path": {
                                    "type": "string",
                                    "description": "분석 결과 JSON 파일 경로"
                                }
                            },
                            "required": ["analysis_result_path"]
                        }
                    )
                ]
            )

        @self.server.call_tool()
        async def handle_call_tool(request: CallToolRequest) -> CallToolResult:
            """도구 호출을 처리합니다."""
            
            if request.name == "analyze_unity_file":
                return await self._analyze_unity_file(request.arguments)
            elif request.name == "analyze_unity_project":
                return await self._analyze_unity_project(request.arguments)
            elif request.name == "get_unity_api_patterns":
                return await self._get_unity_api_patterns()
            elif request.name == "generate_llm_context":
                return await self._generate_llm_context(request.arguments)
            else:
                raise ValueError(f"Unknown tool: {request.name}")

    async def _analyze_unity_file(self, args: Dict[str, Any]) -> CallToolResult:
        """단일 Unity 파일을 분석합니다."""
        file_path = args["file_path"]
        analysis_type = args.get("analysis_type", "detailed")
        
        if not Path(file_path).exists():
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text=f"❌ 파일을 찾을 수 없습니다: {file_path}"
                )]
            )
        
        try:
            # Unity Context Generator 실행
            cmd = [UNITY_CONTEXT_GENERATOR, file_path, "--verbose"]
            if analysis_type == "llm_optimized":
                cmd.extend(["--format", "llm"])
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                # 분석 결과 파일들 읽기
                analysis_dir = Path("unity_context_analysis")
                
                summary = ""
                if (analysis_dir / "summary.md").exists():
                    summary = (analysis_dir / "summary.md").read_text()
                
                detailed_report = ""
                if (analysis_dir / "detailed_report.md").exists():
                    detailed_report = (analysis_dir / "detailed_report.md").read_text()
                
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=f"# Unity 파일 분석 완료\n\n## 파일: {file_path}\n\n{summary}\n\n---\n\n{detailed_report}"
                    )]
                )
            else:
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=f"❌ 분석 실패:\n{result.stderr}"
                    )]
                )
        
        except subprocess.TimeoutExpired:
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text="❌ 분석 시간 초과 (30초)"
                )]
            )
        except Exception as e:
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text=f"❌ 분석 중 오류 발생: {str(e)}"
                )]
            )

    async def _analyze_unity_project(self, args: Dict[str, Any]) -> CallToolResult:
        """Unity 프로젝트 전체를 분석합니다."""
        directory_path = args["directory_path"]
        include_ai = args.get("include_ai_analysis", False)
        
        if not Path(directory_path).exists():
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text=f"❌ 디렉토리를 찾을 수 없습니다: {directory_path}"
                )]
            )
        
        try:
            cmd = [UNITY_CONTEXT_GENERATOR, "--directory", directory_path, "--verbose"]
            if not include_ai:
                # AI 분석 비활성화를 위한 환경 변수 설정
                env = {"ANTHROPIC_API_KEY": ""}
            else:
                env = None
            
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=120, env=env)
            
            if result.returncode == 0:
                # LLM 최적화된 프롬프트 생성
                analysis_dir = Path("unity_context_analysis")
                
                llm_prompt = ""
                if (analysis_dir / "llm_prompt.md").exists():
                    llm_prompt = (analysis_dir / "llm_prompt.md").read_text()
                
                summary = ""
                if (analysis_dir / "summary.md").exists():
                    summary = (analysis_dir / "summary.md").read_text()
                
                # Unity API 사용 분석
                api_analysis = self._extract_api_analysis(result.stdout)
                
                response_text = f"""# Unity 프로젝트 분석 완료 🎮

## 프로젝트: {directory_path}

{summary}

## Unity API 사용 분석
{api_analysis}

## LLM 최적화된 컨텍스트
{llm_prompt}

---

이 분석 결과를 바탕으로 Unity 개발에 대한 질문이나 개선 사항에 대해 문의하세요!
"""
                
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=response_text
                    )]
                )
            else:
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=f"❌ 프로젝트 분석 실패:\n{result.stderr}"
                    )]
                )
        
        except subprocess.TimeoutExpired:
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text="❌ 분석 시간 초과 (2분)"
                )]
            )
        except Exception as e:
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text=f"❌ 분석 중 오류 발생: {str(e)}"
                )]
            )

    async def _get_unity_api_patterns(self) -> CallToolResult:
        """지원되는 Unity API 패턴들을 반환합니다."""
        patterns = """# Unity Smart Context가 인식하는 Unity API 패턴들

## 입력 시스템 (Input APIs)
- `Input.GetAxis()` - 축 입력 값 가져오기
- `Input.GetKey()` - 키 입력 상태 확인
- `Input.GetKeyDown()` - 키 누름 감지
- `Input.GetMouseButton()` - 마우스 버튼 상태

## 컴포넌트 관리 (Component APIs)
- `GetComponent<T>()` - 컴포넌트 참조 (⚠️ 성능 주의)
- `AddComponent<T>()` - 컴포넌트 추가
- `RequireComponent` - 컴포넌트 의존성 선언

## 변형 및 위치 (Transform APIs)
- `transform.position` - 위치 조작
- `transform.rotation` - 회전 조작
- `transform.localScale` - 크기 조작
- `Transform.Translate()` - 이동

## 물리 시스템 (Physics APIs)
- `Rigidbody.AddForce()` - 힘 적용
- `Rigidbody.velocity` - 속도 조작
- `OnCollisionEnter()` - 충돌 감지
- `OnTriggerEnter()` - 트리거 감지

## 렌더링 (Rendering APIs)
- `Renderer.material` - 머티리얼 조작
- `Camera.main` - 메인 카메라 참조
- `SetActive()` - 오브젝트 활성화/비활성화

## 성능에 민감한 API들 ⚠️
- `GetComponent()` - 캐싱 필요
- `Find()` 계열 - 사용 자제
- `SendMessage()` - 리플렉션 사용으로 느림
- `Camera.main` - 매 프레임 호출 시 성능 저하

## 라이프사이클 메서드
- `Awake()` - 초기화 (컴포넌트 참조)
- `Start()` - 시작 로직
- `Update()` - 매 프레임 업데이트
- `FixedUpdate()` - 고정 시간 간격 업데이트 (물리)
- `LateUpdate()` - 다른 업데이트 후 실행
- `OnDestroy()` - 정리 작업
"""
        
        return CallToolResult(
            content=[TextContent(
                type="text",
                text=patterns
            )]
        )

    async def _generate_llm_context(self, args: Dict[str, Any]) -> CallToolResult:
        """분석 결과를 LLM 컨텍스트로 변환합니다."""
        result_path = args["analysis_result_path"]
        
        try:
            if Path(result_path).exists():
                with open(result_path, 'r', encoding='utf-8') as f:
                    analysis_data = json.load(f)
                
                # LLM 친화적 컨텍스트 생성
                context = self._create_llm_context(analysis_data)
                
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=context
                    )]
                )
            else:
                return CallToolResult(
                    content=[TextContent(
                        type="text",
                        text=f"❌ 분석 결과 파일을 찾을 수 없습니다: {result_path}"
                    )]
                )
        except Exception as e:
            return CallToolResult(
                content=[TextContent(
                    type="text",
                    text=f"❌ 컨텍스트 생성 중 오류: {str(e)}"
                )]
            )

    def _extract_api_analysis(self, stdout: str) -> str:
        """stdout에서 Unity API 분석 결과를 추출합니다."""
        lines = stdout.split('\n')
        api_info = []
        
        for line in lines:
            if "UnityAPIDetector" in line and "Detected" in line:
                api_info.append(line)
            elif "Quality Score" in line:
                api_info.append(line)
        
        return '\n'.join(api_info) if api_info else "Unity API 사용 정보를 찾을 수 없습니다."

    def _create_llm_context(self, analysis_data: Dict) -> str:
        """분석 데이터를 LLM 컨텍스트로 변환합니다."""
        return f"""# Unity 프로젝트 LLM 컨텍스트

## 프로젝트 개요
- **타입**: {analysis_data.get('project_type', 'Unity Game')}
- **아키텍처**: {analysis_data.get('architecture', 'Component-based')}
- **품질 점수**: {analysis_data.get('quality_score', 'N/A')}

## 컴포넌트 정보
{json.dumps(analysis_data.get('components', {}), indent=2, ensure_ascii=False)}

## 개발 가이드라인
이 프로젝트는 Unity의 컴포넌트 기반 아키텍처를 따릅니다. 
코드 수정 시 기존 패턴을 유지하고 Unity 최적화 가이드라인을 준수하세요.
"""

    async def run(self):
        """MCP 서버를 실행합니다."""
        async with stdio_server() as (read_stream, write_stream):
            await self.server.run(
                read_stream,
                write_stream,
                InitializationOptions(
                    server_name="unity-context-generator",
                    server_version="1.0.0",
                    capabilities=self.server.get_capabilities(
                        notification_options=NotificationOptions(),
                        experimental_capabilities={},
                    ),
                ),
            )

async def main():
    """메인 실행 함수"""
    server = UnityContextMCPServer()
    await server.run()

if __name__ == "__main__":
    asyncio.run(main())