import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { exec } from 'child_process';
import { promisify } from 'util';

const execAsync = promisify(exec);

export function activate(context: vscode.ExtensionContext) {
    console.log('Unity Smart Context extension is now active!');

    // 현재 파일 분석 명령어
    let analyzeCurrentFile = vscode.commands.registerCommand('unity-smart-context.analyzeCurrentFile', async () => {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('활성화된 에디터가 없습니다.');
            return;
        }

        const document = editor.document;
        if (document.languageId !== 'csharp') {
            vscode.window.showErrorMessage('Unity C# 파일만 분석할 수 있습니다.');
            return;
        }

        await analyzeUnityFile(document.fileName);
    });

    // 워크스페이스 분석 명령어
    let analyzeWorkspace = vscode.commands.registerCommand('unity-smart-context.analyzeWorkspace', async (uri: vscode.Uri) => {
        const folderPath = uri ? uri.fsPath : vscode.workspace.rootPath;
        if (!folderPath) {
            vscode.window.showErrorMessage('워크스페이스가 열려있지 않습니다.');
            return;
        }

        await analyzeUnityProject(folderPath);
    });

    // Unity API 패턴 표시 명령어
    let showPatterns = vscode.commands.registerCommand('unity-smart-context.showPatterns', () => {
        showUnityPatterns();
    });

    // 파일 저장 시 자동 분석
    let onSaveHandler = vscode.workspace.onDidSaveTextDocument(async (document) => {
        const config = vscode.workspace.getConfiguration('unity-smart-context');
        const autoAnalyze = config.get<boolean>('autoAnalyze', false);
        
        if (autoAnalyze && document.languageId === 'csharp' && isUnityFile(document.fileName)) {
            await analyzeUnityFile(document.fileName, false); // 조용히 분석
        }
    });

    context.subscriptions.push(
        analyzeCurrentFile,
        analyzeWorkspace,
        showPatterns,
        onSaveHandler
    );
}

async function analyzeUnityFile(filePath: string, showProgress: boolean = true) {
    const config = vscode.workspace.getConfiguration('unity-smart-context');
    const generatorPath = config.get<string>('generatorPath') || findGeneratorPath();
    const outputFormat = config.get<string>('outputFormat', 'claude');

    if (!generatorPath || !fs.existsSync(generatorPath)) {
        vscode.window.showErrorMessage(
            'Unity Context Generator를 찾을 수 없습니다. 설정에서 경로를 지정해주세요.',
            '설정 열기'
        ).then(selection => {
            if (selection === '설정 열기') {
                vscode.commands.executeCommand('workbench.action.openSettings', 'unity-smart-context.generatorPath');
            }
        });
        return;
    }

    const progressOptions = {
        location: vscode.ProgressLocation.Notification,
        title: "Unity 스크립트 분석 중...",
        cancellable: false
    };

    if (showProgress) {
        return vscode.window.withProgress(progressOptions, async (progress) => {
            return await runAnalysis(generatorPath, filePath, outputFormat, progress);
        });
    } else {
        return await runAnalysis(generatorPath, filePath, outputFormat);
    }
}

async function analyzeUnityProject(directoryPath: string) {
    const config = vscode.workspace.getConfiguration('unity-smart-context');
    const generatorPath = config.get<string>('generatorPath') || findGeneratorPath();
    const outputFormat = config.get<string>('outputFormat', 'claude');

    if (!generatorPath || !fs.existsSync(generatorPath)) {
        vscode.window.showErrorMessage('Unity Context Generator를 찾을 수 없습니다.');
        return;
    }

    const progressOptions = {
        location: vscode.ProgressLocation.Notification,
        title: "Unity 프로젝트 분석 중...",
        cancellable: false
    };

    return vscode.window.withProgress(progressOptions, async (progress) => {
        progress.report({ message: "프로젝트 스캔 중..." });
        
        try {
            const command = `"${generatorPath}" --directory "${directoryPath}" --verbose`;
            const { stdout, stderr } = await execAsync(command, { timeout: 120000 });

            if (stderr && !stderr.includes('warning')) {
                throw new Error(stderr);
            }

            progress.report({ message: "결과 처리 중..." });
            
            // 분석 결과 읽기
            const analysisDir = path.join(process.cwd(), 'unity_context_analysis');
            const results = await readAnalysisResults(analysisDir);

            // Claude 컨텍스트 생성
            const claudeContext = generateProjectClaudeContext(results, directoryPath);

            // 결과 표시
            await showAnalysisResults(claudeContext, `Unity 프로젝트 분석: ${path.basename(directoryPath)}`, results);

            vscode.window.showInformationMessage(
                `Unity 프로젝트 분석이 완료되었습니다! (${results.componentCount || 0}개 컴포넌트 발견)`
            );

        } catch (error) {
            const errorMessage = error instanceof Error ? error.message : '알 수 없는 오류';
            vscode.window.showErrorMessage(`분석 실패: ${errorMessage}`);
        }
    });
}

async function runAnalysis(generatorPath: string, filePath: string, outputFormat: string, progress?: vscode.Progress<{message?: string}>) {
    try {
        if (progress) {
            progress.report({ message: "Unity API 탐지 중..." });
        }

        const command = `"${generatorPath}" "${filePath}" --verbose`;
        const { stdout, stderr } = await execAsync(command, { timeout: 30000 });

        if (stderr && !stderr.includes('warning')) {
            throw new Error(stderr);
        }

        if (progress) {
            progress.report({ message: "결과 처리 중..." });
        }

        // 분석 결과 읽기
        const analysisDir = path.join(process.cwd(), 'unity_context_analysis');
        const results = await readAnalysisResults(analysisDir);

        // 파일 내용 읽기
        const fileContent = fs.readFileSync(filePath, 'utf8');
        const fileName = path.basename(filePath);

        // Claude 컨텍스트 생성
        const claudeContext = generateFileClaudeContext(results, fileContent, fileName);

        // 결과 표시
        await showAnalysisResults(claudeContext, `Unity 파일 분석: ${fileName}`, results, stdout);

        vscode.window.showInformationMessage(
            `${fileName} 분석 완료! Unity API ${results.apiCount || 0}개 탐지됨`
        );

    } catch (error) {
        const errorMessage = error instanceof Error ? error.message : '알 수 없는 오류';
        vscode.window.showErrorMessage(`분석 실패: ${errorMessage}`);
    }
}

async function readAnalysisResults(analysisDir: string): Promise<any> {
    const results: any = {};

    try {
        // 각 결과 파일 읽기
        const files = [
            { key: 'summary', file: 'summary.md' },
            { key: 'detailed', file: 'detailed_report.md' },
            { key: 'llmPrompt', file: 'llm_prompt.md' },
            { key: 'metadata', file: 'project_metadata.json' },
            { key: 'context', file: 'project_context.json' }
        ];

        for (const { key, file } of files) {
            const filePath = path.join(analysisDir, file);
            if (fs.existsSync(filePath)) {
                const content = fs.readFileSync(filePath, 'utf8');
                results[key] = file.endsWith('.json') ? JSON.parse(content) : content;
            }
        }

        // API 개수 추출
        if (results.summary) {
            const apiMatch = results.summary.match(/MonoBehaviours: (\d+)/);
            results.componentCount = apiMatch ? parseInt(apiMatch[1]) : 0;
        }

    } catch (error) {
        console.error('결과 파일 읽기 오류:', error);
    }

    return results;
}

function generateFileClaudeContext(results: any, fileContent: string, fileName: string): string {
    const summary = results.summary || '요약 없음';
    const detailed = results.detailed || '상세 분석 없음';

    return `# Unity 스크립트 분석 결과

## 파일 정보
- **파일명**: ${fileName}
- **분석 도구**: Unity Smart Context Generator

## 코드 요약
${summary}

## 상세 분석
${detailed}

## 원본 코드
\`\`\`csharp
${fileContent.length > 2000 ? fileContent.substring(0, 2000) + '...' : fileContent}
\`\`\`

## Unity 개발 컨텍스트
이 스크립트는 Unity 게임 엔진용 C# 코드입니다. 분석 결과를 바탕으로 다음과 같은 도움을 받을 수 있습니다:

1. **성능 최적화**: "이 코드의 성능을 어떻게 개선할 수 있나요?"
2. **베스트 프랙티스**: "Unity 베스트 프랙티스에 맞게 코드를 수정해주세요"
3. **버그 수정**: "잠재적인 버그나 문제점을 찾아주세요"
4. **기능 확장**: "이 스크립트에 [특정 기능]을 추가하려면 어떻게 해야 하나요?"

분석 도구가 감지한 Unity API와 패턴을 바탕으로 구체적이고 실용적인 조언을 제공할 수 있습니다.`;
}

function generateProjectClaudeContext(results: any, directoryPath: string): string {
    const summary = results.summary || '요약 없음';
    const llmPrompt = results.llmPrompt || 'LLM 프롬프트 없음';

    return `# Unity 프로젝트 전체 분석 결과

## 프로젝트 정보
- **경로**: ${directoryPath}
- **분석 도구**: Unity Smart Context Generator

## 프로젝트 요약
${summary}

## LLM 최적화된 컨텍스트
${llmPrompt}

## 개발 가이드라인
이 Unity 프로젝트에 대한 포괄적인 분석이 완료되었습니다. 다음과 같은 도움을 받을 수 있습니다:

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

구체적인 질문이나 특정 컴포넌트에 대한 개선 요청을 해주시면 더 정확한 도움을 드릴 수 있습니다.`;
}

async function showAnalysisResults(claudeContext: string, title: string, results: any, stdout?: string) {
    // 새 문서 생성하여 결과 표시
    const doc = await vscode.workspace.openTextDocument({
        content: claudeContext,
        language: 'markdown'
    });

    const editor = await vscode.window.showTextDocument(doc);

    // 클립보드에 복사하기 옵션 제공
    const copyOption = 'Claude용 컨텍스트 복사';
    const showDetailsOption = '상세 결과 보기';
    
    const selection = await vscode.window.showInformationMessage(
        title + ' - Claude AI와 함께 사용할 컨텍스트가 생성되었습니다!',
        copyOption,
        showDetailsOption
    );

    if (selection === copyOption) {
        await vscode.env.clipboard.writeText(claudeContext);
        vscode.window.showInformationMessage('클립보드에 복사되었습니다! Claude에게 붙여넣기 하세요.');
    } else if (selection === showDetailsOption) {
        // 상세 결과를 새 탭에 표시
        const detailedContent = `# Unity 분석 상세 결과

${results.detailed || '상세 분석 없음'}

## 원본 분석 출력
\`\`\`
${stdout || '출력 없음'}
\`\`\``;

        const detailedDoc = await vscode.workspace.openTextDocument({
            content: detailedContent,
            language: 'markdown'
        });
        await vscode.window.showTextDocument(detailedDoc, vscode.ViewColumn.Beside);
    }
}

function showUnityPatterns() {
    const patterns = `# Unity Smart Context가 인식하는 Unity API 패턴들

## 입력 시스템 (Input APIs)
- \`Input.GetAxis()\` - 축 입력 값 가져오기
- \`Input.GetKey()\` - 키 입력 상태 확인
- \`Input.GetKeyDown()\` - 키 누름 감지
- \`Input.GetMouseButton()\` - 마우스 버튼 상태

## 컴포넌트 관리 (Component APIs)
- \`GetComponent<T>()\` - 컴포넌트 참조 (⚠️ 성능 주의)
- \`AddComponent<T>()\` - 컴포넌트 추가
- \`RequireComponent\` - 컴포넌트 의존성 선언

## 변형 및 위치 (Transform APIs)
- \`transform.position\` - 위치 조작
- \`transform.rotation\` - 회전 조작
- \`transform.localScale\` - 크기 조작
- \`Transform.Translate()\` - 이동

## 물리 시스템 (Physics APIs)
- \`Rigidbody.AddForce()\` - 힘 적용
- \`Rigidbody.velocity\` - 속도 조작
- \`OnCollisionEnter()\` - 충돌 감지
- \`OnTriggerEnter()\` - 트리거 감지

## 성능에 민감한 API들 ⚠️
- \`GetComponent()\` - 캐싱 필요
- \`Find()\` 계열 - 사용 자제
- \`SendMessage()\` - 리플렉션 사용으로 느림
- \`Camera.main\` - 매 프레임 호출 시 성능 저하

## 라이프사이클 메서드
- \`Awake()\` - 초기화 (컴포넌트 참조)
- \`Start()\` - 시작 로직
- \`Update()\` - 매 프레임 업데이트
- \`FixedUpdate()\` - 고정 시간 간격 업데이트 (물리)
- \`LateUpdate()\` - 다른 업데이트 후 실행
- \`OnDestroy()\` - 정리 작업`;

    vscode.workspace.openTextDocument({
        content: patterns,
        language: 'markdown'
    }).then(doc => {
        vscode.window.showTextDocument(doc);
    });
}

function findGeneratorPath(): string {
    // 일반적인 위치에서 generator 찾기
    const possiblePaths = [
        './build/CLI/unity_context_generator',
        '../build/CLI/unity_context_generator',
        './unity_context_generator',
        'unity_context_generator'
    ];

    for (const p of possiblePaths) {
        if (fs.existsSync(p)) {
            return path.resolve(p);
        }
    }

    return '';
}

function isUnityFile(filePath: string): boolean {
    // Unity 프로젝트 파일인지 간단히 확인
    const content = fs.readFileSync(filePath, 'utf8');
    return content.includes('UnityEngine') || content.includes('MonoBehaviour');
}

export function deactivate() {}