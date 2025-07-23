using System;
using System.IO;
using UnityEngine;
using UnityEditor;

namespace Unity.LLMContextGenerator.Editor
{
    /// <summary>
    /// Unity Editor window for LLM Context Generator
    /// Provides GUI interface for analyzing Unity projects and generating AI-friendly context
    /// </summary>
    public class LLMContextGeneratorWindow : EditorWindow
    {
        #region Fields

        private AnalysisOptions m_analysisOptions;
        private AnalysisResult m_lastResult;
        private bool m_isAnalyzing;
        private float m_analysisProgress;
        private string m_statusMessage = "Ready to analyze";
        
        // GUI State
        private Vector2 m_scrollPosition;
        private bool m_showAdvancedOptions;
        private bool m_showResults;
        private bool m_showLLMPrompt;
        
        // Styles
        private GUIStyle m_headerStyle;
        private GUIStyle m_resultStyle;
        private bool m_stylesInitialized;

        #endregion

        #region Menu Integration

        [MenuItem("Tools/LLM Context Generator")]
        public static void ShowWindow()
        {
            var window = GetWindow<LLMContextGeneratorWindow>("LLM Context Generator");
            window.titleContent = new GUIContent("LLM Context", "Generate AI-friendly context for Unity projects");
            window.minSize = new Vector2(400, 600);
            window.Show();
        }

        #endregion

        #region Unity Lifecycle

        private void OnEnable()
        {
            InitializeOptions();
        }

        private void OnGUI()
        {
            InitializeStyles();
            
            m_scrollPosition = EditorGUILayout.BeginScrollView(m_scrollPosition);
            
            DrawHeader();
            DrawAnalysisOptions();
            DrawAnalysisControls();
            DrawProgressBar();
            DrawResults();
            
            EditorGUILayout.EndScrollView();
        }

        #endregion

        #region GUI Drawing

        private void InitializeStyles()
        {
            if (m_stylesInitialized) return;
            
            m_headerStyle = new GUIStyle(EditorStyles.largeLabel)
            {
                fontSize = 18,
                fontStyle = FontStyle.Bold,
                alignment = TextAnchor.MiddleCenter
            };
            
            m_resultStyle = new GUIStyle(EditorStyles.textArea)
            {
                wordWrap = true,
                richText = true
            };
            
            m_stylesInitialized = true;
        }

        private void DrawHeader()
        {
            EditorGUILayout.Space(10);
            
            // Logo/Header
            EditorGUILayout.LabelField("🤖 Unity LLM Context Generator", m_headerStyle, GUILayout.Height(30));
            EditorGUILayout.LabelField("Generate AI-friendly context metadata for Claude Code and other LLMs", 
                EditorStyles.centeredGreyMiniLabel);
            
            EditorGUILayout.Space(10);
            
            // Quick info
            using (new EditorGUILayout.HorizontalScope(EditorStyles.helpBox))
            {
                EditorGUILayout.LabelField("Project Path:", EditorStyles.boldLabel, GUILayout.Width(100));
                EditorGUILayout.LabelField(Application.dataPath, EditorStyles.wordWrappedLabel);
            }
            
            EditorGUILayout.Space(10);
        }

        private void DrawAnalysisOptions()
        {
            // Basic Options
            EditorGUILayout.LabelField("📊 Analysis Options", EditorStyles.boldLabel);
            
            using (new EditorGUILayout.VerticalScope(EditorStyles.helpBox))
            {
                m_analysisOptions.analyzeDependencies = EditorGUILayout.Toggle(
                    new GUIContent("Analyze Dependencies", "Detect GetComponent calls and component relationships"),
                    m_analysisOptions.analyzeDependencies);
                
                m_analysisOptions.analyzeLifecycleMethods = EditorGUILayout.Toggle(
                    new GUIContent("Analyze Unity Lifecycle", "Detect Unity methods like Awake, Start, Update"),
                    m_analysisOptions.analyzeLifecycleMethods);
                
                m_analysisOptions.detectDesignPatterns = EditorGUILayout.Toggle(
                    new GUIContent("Detect Design Patterns", "Identify common Unity patterns like Singleton, Observer"),
                    m_analysisOptions.detectDesignPatterns);
                
                m_analysisOptions.generateContextSummary = EditorGUILayout.Toggle(
                    new GUIContent("Generate Context Summary", "Create LLM-optimized project overview"),
                    m_analysisOptions.generateContextSummary);
            }
            
            // Advanced Options
            EditorGUILayout.Space(5);
            m_showAdvancedOptions = EditorGUILayout.Foldout(m_showAdvancedOptions, "🔧 Advanced Options");
            
            if (m_showAdvancedOptions)
            {
                using (new EditorGUILayout.VerticalScope(EditorStyles.helpBox))
                {
                    EditorGUILayout.LabelField("Output Formats", EditorStyles.boldLabel);
                    m_analysisOptions.exportJson = EditorGUILayout.Toggle("Export JSON Metadata", m_analysisOptions.exportJson);
                    m_analysisOptions.exportMarkdown = EditorGUILayout.Toggle("Export Markdown Report", m_analysisOptions.exportMarkdown);
                    m_analysisOptions.exportLLMPrompt = EditorGUILayout.Toggle("Export LLM Prompt", m_analysisOptions.exportLLMPrompt);
                    
                    EditorGUILayout.Space(5);
                    EditorGUILayout.LabelField("Debug Options", EditorStyles.boldLabel);
                    m_analysisOptions.verboseLogging = EditorGUILayout.Toggle("Verbose Logging", m_analysisOptions.verboseLogging);
                }
            }
            
            EditorGUILayout.Space(10);
        }

        private void DrawAnalysisControls()
        {
            EditorGUILayout.LabelField("🚀 Analysis Controls", EditorStyles.boldLabel);
            
            using (new EditorGUILayout.HorizontalScope())
            {
                GUI.enabled = !m_isAnalyzing;
                
                if (GUILayout.Button("🔍 Analyze Current Project", GUILayout.Height(35)))
                {
                    StartAnalysis();
                }
                
                if (GUILayout.Button("📋 Generate LLM Prompt", GUILayout.Height(35), GUILayout.Width(150)))
                {
                    if (m_lastResult != null && m_lastResult.Success)
                    {
                        GenerateAndShowPrompt();
                    }
                    else
                    {
                        EditorUtility.DisplayDialog("No Analysis Results", 
                            "Please run an analysis first before generating the LLM prompt.", "OK");
                    }
                }
                
                GUI.enabled = true;
            }
            
            // Status message
            using (new EditorGUILayout.HorizontalScope(EditorStyles.helpBox))
            {
                EditorGUILayout.LabelField("Status:", EditorStyles.boldLabel, GUILayout.Width(50));
                EditorGUILayout.LabelField(m_statusMessage, EditorStyles.wordWrappedLabel);
            }
            
            EditorGUILayout.Space(10);
        }

        private void DrawProgressBar()
        {
            if (m_isAnalyzing)
            {
                EditorGUILayout.LabelField("⏳ Analysis Progress", EditorStyles.boldLabel);
                
                Rect rect = EditorGUILayout.GetControlRect(false, 20);
                EditorGUI.ProgressBar(rect, m_analysisProgress, $"{m_analysisProgress:P0}");
                
                EditorGUILayout.Space(10);
                Repaint();
            }
        }

        private void DrawResults()
        {
            if (m_lastResult == null) return;
            
            EditorGUILayout.LabelField("📈 Analysis Results", EditorStyles.boldLabel);
            
            m_showResults = EditorGUILayout.Foldout(m_showResults, 
                m_lastResult.Success ? "✅ Analysis Successful" : "❌ Analysis Failed", true);
            
            if (m_showResults)
            {
                using (new EditorGUILayout.VerticalScope(EditorStyles.helpBox))
                {
                    if (m_lastResult.Success)
                    {
                        DrawSuccessfulResults();
                    }
                    else
                    {
                        DrawFailedResults();
                    }
                }
            }
            
            EditorGUILayout.Space(10);
        }

        private void DrawSuccessfulResults()
        {
            // Summary stats
            using (new EditorGUILayout.HorizontalScope())
            {
                EditorGUILayout.LabelField($"🎮 MonoBehaviours: {m_lastResult.MonoBehaviourCount}", GUILayout.Width(150));
                EditorGUILayout.LabelField($"🔗 Dependencies: {m_lastResult.DependencyCount}", GUILayout.Width(150));
                EditorGUILayout.LabelField($"📐 Patterns: {m_lastResult.DetectedPatternCount}");
            }
            
            using (new EditorGUILayout.HorizontalScope())
            {
                EditorGUILayout.LabelField($"⚡ Duration: {m_lastResult.AnalysisDurationMs:F1}ms", GUILayout.Width(150));
                EditorGUILayout.LabelField($"🏗️ Architecture: {m_lastResult.Architecture}", GUILayout.Width(150));
                EditorGUILayout.LabelField($"📊 Quality: {m_lastResult.QualityScore}%");
            }
            
            EditorGUILayout.Space(5);
            
            // Action buttons
            using (new EditorGUILayout.HorizontalScope())
            {
                if (GUILayout.Button("📁 Open Output Folder"))
                {
                    OpenOutputFolder();
                }
                
                if (GUILayout.Button("📋 Copy LLM Prompt"))
                {
                    CopyLLMPromptToClipboard();
                }
                
                if (GUILayout.Button("💾 Export Results"))
                {
                    ExportResultsToFile();
                }
            }
            
            // LLM Prompt preview
            EditorGUILayout.Space(5);
            m_showLLMPrompt = EditorGUILayout.Foldout(m_showLLMPrompt, "🤖 LLM Prompt Preview", true);
            
            if (m_showLLMPrompt && !string.IsNullOrEmpty(m_lastResult.LLMPrompt))
            {
                EditorGUILayout.LabelField("Generated prompt for Claude Code:", EditorStyles.boldLabel);
                
                string previewText = m_lastResult.LLMPrompt;
                if (previewText.Length > 500)
                {
                    previewText = previewText.Substring(0, 500) + "...\n\n[Click 'Copy LLM Prompt' to get full text]";
                }
                
                EditorGUILayout.TextArea(previewText, m_resultStyle, GUILayout.MinHeight(100));
            }
        }

        private void DrawFailedResults()
        {
            EditorGUILayout.HelpBox($"Analysis failed: {m_lastResult.ErrorMessage}", MessageType.Error);
            
            if (GUILayout.Button("🔄 Retry Analysis"))
            {
                StartAnalysis();
            }
        }

        #endregion

        #region Analysis Logic

        private void InitializeOptions()
        {
            if (m_analysisOptions == null)
            {
                m_analysisOptions = new AnalysisOptions
                {
                    analyzeDependencies = true,
                    analyzeLifecycleMethods = true,
                    detectDesignPatterns = true,
                    generateContextSummary = true,
                    exportLLMPrompt = true,
                    verboseLogging = false
                };
            }
        }

        private void StartAnalysis()
        {
            if (m_isAnalyzing) return;
            
            m_isAnalyzing = true;
            m_analysisProgress = 0f;
            m_statusMessage = "Starting analysis...";
            
            try
            {
                // Run analysis in background (simulated for now)
                EditorApplication.update += UpdateAnalysisProgress;
                
                // In real implementation, this would call the native library
                m_lastResult = LLMContextAnalyzer.AnalyzeCurrentProject(m_analysisOptions);
                
                m_isAnalyzing = false;
                m_analysisProgress = 1f;
                m_statusMessage = m_lastResult.Success ? "Analysis completed successfully!" : "Analysis failed";
                m_showResults = true;
                
                EditorApplication.update -= UpdateAnalysisProgress;
                
                if (m_lastResult.Success)
                {
                    Debug.Log($"[LLMContextGenerator] Analysis completed: {m_lastResult.MonoBehaviourCount} components found");
                }
            }
            catch (Exception e)
            {
                m_isAnalyzing = false;
                m_statusMessage = $"Analysis error: {e.Message}";
                m_lastResult = new AnalysisResult { Success = false, ErrorMessage = e.Message };
                
                EditorApplication.update -= UpdateAnalysisProgress;
                Debug.LogError($"[LLMContextGenerator] Analysis failed: {e}");
            }
        }

        private void UpdateAnalysisProgress()
        {
            if (m_isAnalyzing)
            {
                m_analysisProgress = Mathf.Min(m_analysisProgress + 0.02f, 0.95f);
            }
        }

        private void GenerateAndShowPrompt()
        {
            if (m_lastResult == null || !m_lastResult.Success) return;
            
            try
            {
                string prompt = LLMContextAnalyzer.GeneratePromptFromResult(m_lastResult);
                
                if (!string.IsNullOrEmpty(prompt))
                {
                    m_lastResult.LLMPrompt = prompt;
                    m_showLLMPrompt = true;
                    m_statusMessage = "LLM prompt generated successfully";
                }
                else
                {
                    m_statusMessage = "Failed to generate LLM prompt";
                }
            }
            catch (Exception e)
            {
                m_statusMessage = $"Prompt generation error: {e.Message}";
                Debug.LogError($"[LLMContextGenerator] Prompt generation failed: {e}");
            }
        }

        #endregion

        #region Utility Methods

        private void OpenOutputFolder()
        {
            string outputPath = Path.Combine(Application.dataPath, "../unity_context_analysis");
            if (Directory.Exists(outputPath))
            {
                EditorUtility.RevealInFinder(outputPath);
            }
            else
            {
                EditorUtility.DisplayDialog("Output Folder Not Found", 
                    $"The output folder does not exist yet.\nRun an analysis to generate output files.", "OK");
            }
        }

        private void CopyLLMPromptToClipboard()
        {
            if (m_lastResult != null && !string.IsNullOrEmpty(m_lastResult.LLMPrompt))
            {
                GUIUtility.systemCopyBuffer = m_lastResult.LLMPrompt;
                m_statusMessage = "LLM prompt copied to clipboard";
                
                EditorUtility.DisplayDialog("Copied to Clipboard", 
                    "The LLM prompt has been copied to your clipboard.\nYou can now paste it into Claude Code or other AI tools.", "OK");
            }
        }

        private void ExportResultsToFile()
        {
            if (m_lastResult == null || !m_lastResult.Success) return;
            
            string fileName = $"unity_llm_context_{DateTime.Now:yyyy-MM-dd_HH-mm-ss}.json";
            string filePath = EditorUtility.SaveFilePanel("Export Analysis Results", 
                Application.dataPath, fileName, "json");
            
            if (!string.IsNullOrEmpty(filePath))
            {
                try
                {
                    string json = JsonUtility.ToJson(m_lastResult, true);
                    File.WriteAllText(filePath, json);
                    
                    m_statusMessage = "Results exported successfully";
                    EditorUtility.DisplayDialog("Export Successful", 
                        $"Analysis results exported to:\n{filePath}", "OK");
                }
                catch (Exception e)
                {
                    m_statusMessage = $"Export failed: {e.Message}";
                    EditorUtility.DisplayDialog("Export Failed", 
                        $"Failed to export results:\n{e.Message}", "OK");
                }
            }
        }

        #endregion
    }
}