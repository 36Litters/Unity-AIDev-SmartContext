#!/usr/bin/env python3
"""
Unity Smart Context Web Interface
웹 브라우저에서 Claude와 함께 사용할 수 있는 인터페이스
"""

from flask import Flask, render_template, request, jsonify, send_file
import subprocess
import json
import os
from pathlib import Path
import tempfile
import zipfile

app = Flask(__name__)

# Unity Context Generator 실행 파일 경로
UNITY_CONTEXT_GENERATOR = "../build/CLI/unity_context_generator"

@app.route('/')
def index():
    """메인 페이지"""
    return render_template('index.html')

@app.route('/api/analyze', methods=['POST'])
def analyze_unity_code():
    """Unity 코드 분석 API"""
    try:
        data = request.get_json()
        
        if 'code' in data:
            # 코드 문자열 분석
            return analyze_code_string(data['code'], data.get('filename', 'script.cs'))
        elif 'file_path' in data:
            # 파일 경로 분석  
            return analyze_file_path(data['file_path'])
        elif 'directory_path' in data:
            # 디렉토리 분석
            return analyze_directory_path(data['directory_path'])
        else:
            return jsonify({'error': '코드나 파일 경로를 제공해주세요'}), 400
            
    except Exception as e:
        return jsonify({'error': str(e)}), 500

def analyze_code_string(code: str, filename: str):
    """코드 문자열을 분석합니다"""
    with tempfile.NamedTemporaryFile(mode='w', suffix='.cs', delete=False) as f:
        f.write(code)
        temp_file = f.name
    
    try:
        result = subprocess.run(
            [UNITY_CONTEXT_GENERATOR, temp_file, '--verbose'],
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode == 0:
            # 분석 결과 읽기
            analysis_result = read_analysis_results()
            
            # Claude용 컨텍스트 생성
            claude_context = generate_claude_context(analysis_result, code, filename)
            
            return jsonify({
                'success': True,
                'analysis': analysis_result,
                'claude_context': claude_context,
                'stdout': result.stdout
            })
        else:
            return jsonify({
                'success': False,
                'error': result.stderr,
                'stdout': result.stdout
            }), 400
            
    finally:
        os.unlink(temp_file)

def analyze_file_path(file_path: str):
    """파일 경로를 분석합니다"""
    if not Path(file_path).exists():
        return jsonify({'error': f'파일을 찾을 수 없습니다: {file_path}'}), 404
    
    try:
        result = subprocess.run(
            [UNITY_CONTEXT_GENERATOR, file_path, '--verbose'],
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode == 0:
            analysis_result = read_analysis_results()
            
            # 파일 내용 읽기
            with open(file_path, 'r', encoding='utf-8') as f:
                code_content = f.read()
            
            claude_context = generate_claude_context(
                analysis_result, 
                code_content, 
                Path(file_path).name
            )
            
            return jsonify({
                'success': True,
                'analysis': analysis_result,
                'claude_context': claude_context,
                'stdout': result.stdout
            })
        else:
            return jsonify({
                'success': False,
                'error': result.stderr,
                'stdout': result.stdout
            }), 400
            
    except subprocess.TimeoutExpired:
        return jsonify({'error': '분석 시간 초과'}), 408
    except Exception as e:
        return jsonify({'error': str(e)}), 500

def analyze_directory_path(directory_path: str):
    """디렉토리를 분석합니다"""
    if not Path(directory_path).exists():
        return jsonify({'error': f'디렉토리를 찾을 수 없습니다: {directory_path}'}), 404
    
    try:
        result = subprocess.run(
            [UNITY_CONTEXT_GENERATOR, '--directory', directory_path, '--verbose'],
            capture_output=True,
            text=True,
            timeout=120
        )
        
        if result.returncode == 0:
            analysis_result = read_analysis_results()
            
            # 프로젝트용 Claude 컨텍스트 생성
            claude_context = generate_project_claude_context(analysis_result, directory_path)
            
            return jsonify({
                'success': True,
                'analysis': analysis_result,
                'claude_context': claude_context,
                'stdout': result.stdout
            })
        else:
            return jsonify({
                'success': False,
                'error': result.stderr,
                'stdout': result.stdout
            }), 400
            
    except subprocess.TimeoutExpired:
        return jsonify({'error': '분석 시간 초과 (2분)'}), 408
    except Exception as e:
        return jsonify({'error': str(e)}), 500

def read_analysis_results():
    """분석 결과 파일들을 읽어서 통합합니다"""
    analysis_dir = Path("unity_context_analysis")
    results = {}
    
    # 각 분석 결과 파일 읽기
    file_mappings = {
        'summary': 'summary.md',
        'detailed_report': 'detailed_report.md',
        'llm_prompt': 'llm_prompt.md',
        'project_metadata': 'project_metadata.json',
        'project_context': 'project_context.json',
        'llm_optimized': 'llm_optimized.json'
    }
    
    for key, filename in file_mappings.items():
        file_path = analysis_dir / filename
        if file_path.exists():
            try:
                if filename.endswith('.json'):
                    with open(file_path, 'r', encoding='utf-8') as f:
                        results[key] = json.load(f)
                else:
                    with open(file_path, 'r', encoding='utf-8') as f:
                        results[key] = f.read()
            except Exception as e:
                results[key] = f"파일 읽기 오류: {str(e)}"
    
    return results

def generate_claude_context(analysis_result, code_content, filename):
    """Claude용 컨텍스트를 생성합니다"""
    context = f"""# Unity 스크립트 분석 결과

## 파일 정보
- **파일명**: {filename}
- **분석 시간**: {analysis_result.get('summary', '').split('**Analysis Duration:**')[1].split('ms')[0] if '**Analysis Duration:**' in analysis_result.get('summary', '') else 'N/A'}ms

## 코드 요약
{analysis_result.get('summary', '요약 정보 없음')}

## 상세 분석
{analysis_result.get('detailed_report', '상세 분석 없음')}

## 원본 코드
```csharp
{code_content[:2000]}{'...' if len(code_content) > 2000 else ''}
```

## Unity 개발 컨텍스트
이 스크립트는 Unity 게임 엔진용 C# 코드입니다. 분석 결과를 바탕으로 다음과 같은 질문을 할 수 있습니다:

1. **성능 최적화**: "이 코드의 성능을 어떻게 개선할 수 있나요?"
2. **베스트 프랙티스**: "Unity 베스트 프랙티스에 맞게 코드를 수정해주세요"
3. **버그 수정**: "잠재적인 버그나 문제점을 찾아주세요"
4. **기능 확장**: "이 스크립트에 [특정 기능]을 추가하려면 어떻게 해야 하나요?"

분석 도구가 감지한 Unity API와 패턴을 바탕으로 구체적이고 실용적인 조언을 제공할 수 있습니다.
"""
    
    return context

def generate_project_claude_context(analysis_result, directory_path):
    """프로젝트 전체에 대한 Claude 컨텍스트를 생성합니다"""
    context = f"""# Unity 프로젝트 전체 분석 결과

## 프로젝트 정보
- **경로**: {directory_path}
- **분석 대상**: Unity C# 스크립트들

## 프로젝트 요약
{analysis_result.get('summary', '요약 정보 없음')}

## 아키텍처 분석
{analysis_result.get('detailed_report', '상세 분석 없음')}

## LLM 최적화된 컨텍스트
{analysis_result.get('llm_prompt', 'LLM 프롬프트 없음')}

## 개발 가이드라인
이 Unity 프로젝트에 대한 분석이 완료되었습니다. 다음과 같은 도움을 받을 수 있습니다:

### 🎮 Unity 개발 관련
- 컴포넌트 아키텍처 개선
- 성능 최적화 방안
- Unity 라이프사이클 활용법
- 디자인 패턴 적용

### 🔧 코드 품질 개선
- 코드 리팩토링 제안
- 버그 방지 패턴
- 메모리 최적화
- 베스트 프랙티스 적용

### 📊 프로젝트 관리
- 의존성 관리
- 모듈화 방안
- 테스트 전략
- CI/CD 통합

구체적인 질문이나 특정 컴포넌트에 대한 개선 요청을 해주시면 더 정확한 도움을 드릴 수 있습니다.
"""
    
    return context

@app.route('/api/download_results')
def download_results():
    """분석 결과를 ZIP 파일로 다운로드"""
    try:
        analysis_dir = Path("unity_context_analysis")
        if not analysis_dir.exists():
            return jsonify({'error': '분석 결과가 없습니다'}), 404
        
        # ZIP 파일 생성
        with tempfile.NamedTemporaryFile(suffix='.zip', delete=False) as temp_zip:
            with zipfile.ZipFile(temp_zip.name, 'w') as zipf:
                for file_path in analysis_dir.glob("*"):
                    if file_path.is_file():
                        zipf.write(file_path, file_path.name)
            
            return send_file(
                temp_zip.name,
                as_attachment=True,
                download_name='unity_analysis_results.zip',
                mimetype='application/zip'
            )
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/unity_patterns')
def get_unity_patterns():
    """지원되는 Unity API 패턴 목록"""
    patterns = {
        "input_apis": [
            "Input.GetAxis()",
            "Input.GetKey()",
            "Input.GetKeyDown()",
            "Input.GetMouseButton()"
        ],
        "component_apis": [
            "GetComponent<T>()",
            "AddComponent<T>()",
            "RequireComponent"
        ],
        "transform_apis": [
            "transform.position",
            "transform.rotation",
            "transform.localScale",
            "Transform.Translate()"
        ],
        "physics_apis": [
            "Rigidbody.AddForce()",
            "Rigidbody.velocity",
            "OnCollisionEnter()",
            "OnTriggerEnter()"
        ],
        "performance_sensitive": [
            "GetComponent()",
            "Find()",
            "SendMessage()",
            "Camera.main"
        ],
        "lifecycle_methods": [
            "Awake()",
            "Start()",
            "Update()",
            "FixedUpdate()",
            "LateUpdate()",
            "OnDestroy()"
        ]
    }
    
    return jsonify(patterns)

if __name__ == '__main__':
    app.run(debug=True, host='localhost', port=5000)