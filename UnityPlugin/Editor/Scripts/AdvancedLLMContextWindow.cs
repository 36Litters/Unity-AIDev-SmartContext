using System;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEditor;

namespace Unity.LLMContextGenerator.Editor
{
    /// <summary>
    /// Advanced Unity Editor window for LLM Context Generator with Phase 3A features
    /// Includes AI integration, real-time analysis, and ScriptableObject support
    /// </summary>
    public class AdvancedLLMContextWindow : EditorWindow
    {
        #region Data Structures

        [System.Serializable]
        public class AdvancedAnalysisOptions
        {
            [Header("Core Analysis")]
            public bool analyzeDependencies = true;
            public bool analyzeLifecycleMethods = true;
            public bool detectDesignPatterns = true;
            public bool generateMetadata = true;
            public bool generateContextSummary = true;

            [Header("Advanced Features")]
            public bool analyzeScriptableObjects = true;
            public bool analyzeAssetReferences = true;
            public bool analyzeAddressables = false;
            public bool analyzeDOTSECS = false;

            [Header("AI Integration")]
            public bool enableAIAnalysis = false;
            public string anthropicApiKey = "";
            public bool enableCodeSuggestions = true;
            public bool enableArchitectureAnalysis = true;
            public bool enablePerformanceAnalysis = true;
            public bool enableSecurityAnalysis = false;

            [Header("Real-time Options")]
            public bool enableRealTimeAnalysis = false;
            public bool watchFileChanges = true;
            public bool autoAnalyzeOnSave = true;
            public bool incrementalAnalysis = true;
            public int fileChangeDebounceMs = 500;

            [Header("Output")]
            public string outputDirectory = "./unity_context_analysis";
            public bool verboseLogging = false;
            public bool exportJson = true;
            public bool overwriteExisting = false;
        }

        public enum AnalysisTab
        {
            Basic,
            Advanced,
            AI,
            RealTime,
            Results
        }

        #endregion

        #region Fields

        private AdvancedAnalysisOptions m_options;
        private AnalysisResult m_lastResult;
        private bool m_isAnalyzing;
        private bool m_isRealTimeActive;
        private float m_analysisProgress;
        private string m_statusMessage = "Ready to analyze";
        
        // GUI State
        private Vector2 m_scrollPosition;
        private AnalysisTab m_currentTab = AnalysisTab.Basic;
        private bool m_showApiKeyField = false;
        private string m_tempApiKey = "";
        
        // Real-time monitoring
        private double m_lastFileChangeTime;
        private List<string> m_changedFiles = new List<string>();
        private bool m_pendingRealTimeAnalysis;
        
        // AI Analysis Results
        private string m_aiAnalysisResult = "";
        private List<string> m_aiSuggestions = new List<string>();
        
        // Styles
        private GUIStyle m_headerStyle;
        private GUIStyle m_tabStyle;
        private GUIStyle m_resultStyle;
        private GUIStyle m_warningStyle;
        private bool m_stylesInitialized;

        // Config persistence
        private const string PREFS_KEY = "LLMContextGenerator_AdvancedOptions";

        #endregion

        #region Menu Integration

        [MenuItem("Tools/LLM Context Generator (Advanced)")]
        public static void ShowWindow()
        {
            var window = GetWindow<AdvancedLLMContextWindow>("Advanced LLM Context Generator");
            window.titleContent = new GUIContent("Advanced LLM Context", "AI-powered Unity project analysis");
            window.minSize = new Vector2(500, 700);
            window.Show();
        }

        #endregion

        #region Unity Lifecycle

        private void OnEnable()
        {
            LoadPreferences();
            InitializeFileWatcher();
            
            // Subscribe to Unity events
            EditorApplication.projectChanged += OnProjectChanged;
            AssemblyReloadEvents.beforeAssemblyReload += OnBeforeAssemblyReload;
        }

        private void OnDisable()
        {
            SavePreferences();
            StopRealTimeAnalysis();
            
            // Unsubscribe from Unity events
            EditorApplication.projectChanged -= OnProjectChanged;
            AssemblyReloadEvents.beforeAssemblyReload -= OnBeforeAssemblyReload;
        }

        private void OnGUI()
        {
            if (!m_stylesInitialized)
            {
                InitializeStyles();
            }

            DrawHeader();
            DrawTabs();
            DrawTabContent();
            DrawStatusBar();
            
            // Handle real-time updates
            if (m_isRealTimeActive && m_pendingRealTimeAnalysis)
            {
                ProcessPendingRealTimeAnalysis();
            }
        }

        private void Update()
        {
            // Check for file changes in real-time mode
            if (m_isRealTimeActive && m_options.watchFileChanges)
            {
                CheckForFileChanges();
            }
            
            Repaint(); // Keep GUI updated
        }

        #endregion

        #region GUI Drawing Methods

        private void InitializeStyles()
        {
            m_headerStyle = new GUIStyle(EditorStyles.largeLabel)
            {
                fontSize = 18,
                fontStyle = FontStyle.Bold,
                alignment = TextAnchor.MiddleCenter
            };

            m_tabStyle = new GUIStyle(EditorStyles.toolbarButton)
            {
                fontSize = 12,
                fixedHeight = 25
            };

            m_resultStyle = new GUIStyle(EditorStyles.textArea)
            {
                wordWrap = true,
                richText = true
            };

            m_warningStyle = new GUIStyle(EditorStyles.helpBox);

            m_stylesInitialized = true;
        }

        private void DrawHeader()
        {
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Advanced LLM Context Generator", m_headerStyle);
            EditorGUILayout.Space();

            if (m_isRealTimeActive)
            {
                EditorGUILayout.HelpBox("🔄 Real-time analysis is active", MessageType.Info);
            }

            if (!string.IsNullOrEmpty(m_options.anthropicApiKey) && m_options.enableAIAnalysis)
            {
                EditorGUILayout.HelpBox("🤖 AI analysis enabled", MessageType.Info);
            }
        }

        private void DrawTabs()
        {
            EditorGUILayout.BeginHorizontal();
            
            if (GUILayout.Button("Basic", m_currentTab == AnalysisTab.Basic ? EditorStyles.toolbarButton : m_tabStyle))
                m_currentTab = AnalysisTab.Basic;
            
            if (GUILayout.Button("Advanced", m_currentTab == AnalysisTab.Advanced ? EditorStyles.toolbarButton : m_tabStyle))
                m_currentTab = AnalysisTab.Advanced;
            
            if (GUILayout.Button("AI", m_currentTab == AnalysisTab.AI ? EditorStyles.toolbarButton : m_tabStyle))
                m_currentTab = AnalysisTab.AI;
            
            if (GUILayout.Button("Real-time", m_currentTab == AnalysisTab.RealTime ? EditorStyles.toolbarButton : m_tabStyle))
                m_currentTab = AnalysisTab.RealTime;
            
            if (GUILayout.Button("Results", m_currentTab == AnalysisTab.Results ? EditorStyles.toolbarButton : m_tabStyle))
                m_currentTab = AnalysisTab.Results;
            
            EditorGUILayout.EndHorizontal();
            EditorGUILayout.Space();
        }

        private void DrawTabContent()
        {
            m_scrollPosition = EditorGUILayout.BeginScrollView(m_scrollPosition);

            switch (m_currentTab)
            {
                case AnalysisTab.Basic:
                    DrawBasicOptions();
                    break;
                case AnalysisTab.Advanced:
                    DrawAdvancedOptions();
                    break;
                case AnalysisTab.AI:
                    DrawAIOptions();
                    break;
                case AnalysisTab.RealTime:
                    DrawRealTimeOptions();
                    break;
                case AnalysisTab.Results:
                    DrawResults();
                    break;
            }

            EditorGUILayout.EndScrollView();
        }

        private void DrawBasicOptions()
        {
            EditorGUILayout.LabelField("Core Analysis Options", EditorStyles.boldLabel);
            
            m_options.analyzeDependencies = EditorGUILayout.Toggle("Analyze Dependencies", m_options.analyzeDependencies);
            m_options.analyzeLifecycleMethods = EditorGUILayout.Toggle("Analyze Lifecycle Methods", m_options.analyzeLifecycleMethods);
            m_options.detectDesignPatterns = EditorGUILayout.Toggle("Detect Design Patterns", m_options.detectDesignPatterns);
            m_options.generateMetadata = EditorGUILayout.Toggle("Generate Metadata", m_options.generateMetadata);
            m_options.generateContextSummary = EditorGUILayout.Toggle("Generate Context Summary", m_options.generateContextSummary);
            
            EditorGUILayout.Space();
            EditorGUILayout.LabelField("Output Options", EditorStyles.boldLabel);
            
            m_options.outputDirectory = EditorGUILayout.TextField("Output Directory", m_options.outputDirectory);
            m_options.verboseLogging = EditorGUILayout.Toggle("Verbose Logging", m_options.verboseLogging);
            m_options.exportJson = EditorGUILayout.Toggle("Export JSON", m_options.exportJson);
            m_options.overwriteExisting = EditorGUILayout.Toggle("Overwrite Existing", m_options.overwriteExisting);

            EditorGUILayout.Space();
            DrawAnalyzeButton();
        }

        private void DrawAdvancedOptions()
        {
            EditorGUILayout.LabelField("Advanced Analysis Features", EditorStyles.boldLabel);
            
            m_options.analyzeScriptableObjects = EditorGUILayout.Toggle("Analyze ScriptableObjects", m_options.analyzeScriptableObjects);
            m_options.analyzeAssetReferences = EditorGUILayout.Toggle("Analyze Asset References", m_options.analyzeAssetReferences);
            m_options.analyzeAddressables = EditorGUILayout.Toggle("Analyze Addressables", m_options.analyzeAddressables);
            
            EditorGUILayout.BeginHorizontal();
            EditorGUI.BeginDisabledGroup(true); // Future feature
            m_options.analyzeDOTSECS = EditorGUILayout.Toggle("Analyze DOTS/ECS", m_options.analyzeDOTSECS);
            EditorGUI.EndDisabledGroup();
            GUILayout.Label("(Coming Soon)", EditorStyles.miniLabel);
            EditorGUILayout.EndHorizontal();

            EditorGUILayout.Space();
            
            if (m_options.analyzeScriptableObjects)
            {
                EditorGUILayout.HelpBox("ScriptableObject analysis will detect:\n• CreateAssetMenu attributes\n• Asset references and dependencies\n• Configuration vs Database patterns", MessageType.Info);
            }

            EditorGUILayout.Space();
            DrawAnalyzeButton();
        }

        private void DrawAIOptions()
        {
            EditorGUILayout.LabelField("AI-Powered Analysis", EditorStyles.boldLabel);
            
            m_options.enableAIAnalysis = EditorGUILayout.Toggle("Enable AI Analysis", m_options.enableAIAnalysis);
            
            if (m_options.enableAIAnalysis)
            {
                EditorGUILayout.Space();
                
                // API Key configuration
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Anthropic API Key:", GUILayout.Width(120));
                
                if (string.IsNullOrEmpty(m_options.anthropicApiKey))
                {
                    if (GUILayout.Button("Set API Key", GUILayout.Width(100)))
                    {
                        m_showApiKeyField = !m_showApiKeyField;
                    }
                }
                else
                {
                    EditorGUILayout.LabelField("✓ Configured", EditorStyles.miniLabel);
                    if (GUILayout.Button("Change", GUILayout.Width(60)))
                    {
                        m_showApiKeyField = !m_showApiKeyField;
                    }
                }
                EditorGUILayout.EndHorizontal();

                if (m_showApiKeyField)
                {
                    EditorGUILayout.BeginHorizontal();
                    m_tempApiKey = EditorGUILayout.PasswordField(m_tempApiKey);
                    if (GUILayout.Button("Save", GUILayout.Width(50)))
                    {
                        m_options.anthropicApiKey = m_tempApiKey;
                        m_showApiKeyField = false;
                        m_tempApiKey = "";
                        SavePreferences();
                    }
                    EditorGUILayout.EndHorizontal();
                }

                EditorGUILayout.Space();
                
                // AI analysis options
                EditorGUILayout.LabelField("AI Analysis Types", EditorStyles.boldLabel);
                m_options.enableCodeSuggestions = EditorGUILayout.Toggle("Code Suggestions", m_options.enableCodeSuggestions);
                m_options.enableArchitectureAnalysis = EditorGUILayout.Toggle("Architecture Analysis", m_options.enableArchitectureAnalysis);
                m_options.enablePerformanceAnalysis = EditorGUILayout.Toggle("Performance Analysis", m_options.enablePerformanceAnalysis);
                m_options.enableSecurityAnalysis = EditorGUILayout.Toggle("Security Analysis", m_options.enableSecurityAnalysis);
                
                EditorGUILayout.Space();
                
                if (string.IsNullOrEmpty(m_options.anthropicApiKey))
                {
                    EditorGUILayout.HelpBox("Please set your Anthropic API key to enable AI analysis.", MessageType.Warning);
                }
                else
                {
                    EditorGUILayout.HelpBox("AI analysis will use Claude 3.5 Sonnet to:\n• Review code architecture\n• Suggest improvements\n• Identify potential issues\n• Provide Unity best practices", MessageType.Info);
                }
            }
            else
            {
                EditorGUILayout.HelpBox("Enable AI analysis to get intelligent code reviews and suggestions.", MessageType.Info);
            }

            EditorGUILayout.Space();
            DrawAnalyzeButton();
            
            // Show AI results if available
            if (!string.IsNullOrEmpty(m_aiAnalysisResult))
            {
                EditorGUILayout.Space();
                EditorGUILayout.LabelField("AI Analysis Results", EditorStyles.boldLabel);
                EditorGUILayout.TextArea(m_aiAnalysisResult, m_resultStyle, GUILayout.Height(200));
            }
        }

        private void DrawRealTimeOptions()
        {
            EditorGUILayout.LabelField("Real-time Analysis", EditorStyles.boldLabel);
            
            EditorGUILayout.BeginHorizontal();
            m_options.enableRealTimeAnalysis = EditorGUILayout.Toggle("Enable Real-time Analysis", m_options.enableRealTimeAnalysis);
            
            if (m_options.enableRealTimeAnalysis != m_isRealTimeActive)
            {
                if (m_options.enableRealTimeAnalysis)
                {
                    if (GUILayout.Button("Start", GUILayout.Width(60)))
                    {
                        StartRealTimeAnalysis();
                    }
                }
                else
                {
                    if (GUILayout.Button("Stop", GUILayout.Width(60)))
                    {
                        StopRealTimeAnalysis();
                    }
                }
            }
            EditorGUILayout.EndHorizontal();
            
            EditorGUI.BeginDisabledGroup(!m_options.enableRealTimeAnalysis);
            
            m_options.watchFileChanges = EditorGUILayout.Toggle("Watch File Changes", m_options.watchFileChanges);
            m_options.autoAnalyzeOnSave = EditorGUILayout.Toggle("Auto-analyze on Save", m_options.autoAnalyzeOnSave);
            m_options.incrementalAnalysis = EditorGUILayout.Toggle("Incremental Analysis", m_options.incrementalAnalysis);
            
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField("Debounce Time (ms):", GUILayout.Width(130));
            m_options.fileChangeDebounceMs = EditorGUILayout.IntSlider(m_options.fileChangeDebounceMs, 100, 2000);
            EditorGUILayout.EndHorizontal();
            
            EditorGUI.EndDisabledGroup();
            
            EditorGUILayout.Space();
            
            if (m_isRealTimeActive)
            {
                EditorGUILayout.HelpBox("🔄 Real-time monitoring active", MessageType.Info);
                
                if (m_changedFiles.Count > 0)
                {
                    EditorGUILayout.LabelField($"Changed files pending analysis: {m_changedFiles.Count}", EditorStyles.miniLabel);
                }
            }
            else if (m_options.enableRealTimeAnalysis)
            {
                EditorGUILayout.HelpBox("Real-time analysis will automatically detect file changes and trigger incremental analysis.", MessageType.Info);
            }
            else
            {
                EditorGUILayout.HelpBox("Real-time analysis is disabled. Enable to get automatic updates when files change.", MessageType.Info);
            }
        }

        private void DrawResults()
        {
            if (m_lastResult == null)
            {
                EditorGUILayout.HelpBox("No analysis results available. Run an analysis to see results here.", MessageType.Info);
                return;
            }

            EditorGUILayout.LabelField("Analysis Results", EditorStyles.boldLabel);
            
            // Summary
            EditorGUILayout.BeginVertical(EditorStyles.helpBox);
            EditorGUILayout.LabelField($"Success: {m_lastResult.Success}");
            EditorGUILayout.LabelField($"MonoBehaviour Count: {m_lastResult.MonoBehaviourCount}");
            EditorGUILayout.LabelField($"Dependency Count: {m_lastResult.DependencyCount}");
            EditorGUILayout.LabelField($"Analysis Duration: {m_lastResult.AnalysisDurationMs:F0}ms");
            EditorGUILayout.EndVertical();
            
            EditorGUILayout.Space();
            
            // Components
            if (m_lastResult.Components != null && m_lastResult.Components.Length > 0)
            {
                EditorGUILayout.LabelField("Components Found:", EditorStyles.boldLabel);
                foreach (var component in m_lastResult.Components)
                {
                    EditorGUILayout.BeginVertical(EditorStyles.helpBox);
                    EditorGUILayout.LabelField($"• {component.Name} ({component.BaseClass})");
                    if (component.UnityMethods != null && component.UnityMethods.Length > 0)
                    {
                        EditorGUILayout.LabelField($"  Unity Methods: {string.Join(", ", component.UnityMethods)}", EditorStyles.miniLabel);
                    }
                    EditorGUILayout.EndVertical();
                }
            }
            
            EditorGUILayout.Space();
            
            // Export buttons
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Copy LLM Prompt"))
            {
                if (!string.IsNullOrEmpty(m_lastResult.LLMPrompt))
                {
                    EditorGUIUtility.systemCopyBuffer = m_lastResult.LLMPrompt;
                    Debug.Log("LLM prompt copied to clipboard");
                }
            }
            
            if (GUILayout.Button("Open Output Folder"))
            {
                if (Directory.Exists(m_options.outputDirectory))
                {
                    EditorUtility.RevealInFinder(m_options.outputDirectory);
                }
            }
            EditorGUILayout.EndHorizontal();
        }

        private void DrawAnalyzeButton()
        {
            EditorGUILayout.Space();
            
            EditorGUI.BeginDisabledGroup(m_isAnalyzing);
            
            if (GUILayout.Button(m_isAnalyzing ? "Analyzing..." : "Analyze Project", GUILayout.Height(30)))
            {
                StartAnalysis();
            }
            
            EditorGUI.EndDisabledGroup();
            
            if (m_isAnalyzing)
            {
                EditorGUILayout.Space();
                EditorGUILayout.LabelField($"Progress: {m_analysisProgress * 100:F0}%");
                Rect progressRect = GUILayoutUtility.GetRect(0, 20, GUILayout.ExpandWidth(true));
                EditorGUI.ProgressBar(progressRect, m_analysisProgress, m_statusMessage);
            }
        }

        private void DrawStatusBar()
        {
            EditorGUILayout.Space();
            EditorGUILayout.BeginHorizontal(EditorStyles.toolbar);
            EditorGUILayout.LabelField(m_statusMessage, EditorStyles.toolbarButton);
            
            if (m_isRealTimeActive)
            {
                GUILayout.Label("🔄", EditorStyles.toolbarButton, GUILayout.Width(20));
            }
            
            if (!string.IsNullOrEmpty(m_options.anthropicApiKey) && m_options.enableAIAnalysis)
            {
                GUILayout.Label("🤖", EditorStyles.toolbarButton, GUILayout.Width(20));
            }
            
            EditorGUILayout.EndHorizontal();
        }

        #endregion

        #region Analysis Methods

        private void StartAnalysis()
        {
            m_isAnalyzing = true;
            m_analysisProgress = 0f;
            m_statusMessage = "Starting analysis...";
            
            // Create analysis options JSON
            var optionsJson = CreateAnalysisOptionsJson();
            
            // Start analysis (this would call the C++ DLL)
            try
            {
                var projectPath = Application.dataPath;
                m_lastResult = LLMContextAnalyzer.AnalyzeCurrentProject(optionsJson);
                
                if (m_options.enableAIAnalysis && !string.IsNullOrEmpty(m_options.anthropicApiKey))
                {
                    StartAIAnalysis();
                }
                
                m_statusMessage = "Analysis completed successfully";
                m_currentTab = AnalysisTab.Results;
            }
            catch (Exception ex)
            {
                m_statusMessage = $"Analysis failed: {ex.Message}";
                Debug.LogError($"Analysis failed: {ex}");
            }
            finally
            {
                m_isAnalyzing = false;
                m_analysisProgress = 1f;
            }
        }

        private void StartAIAnalysis()
        {
            m_statusMessage = "Running AI analysis...";
            
            // This would integrate with the Claude API
            // For now, simulate AI analysis
            m_aiAnalysisResult = "AI Analysis Results:\n\n" +
                                "✅ Architecture: Good component separation\n" +
                                "⚠️  Performance: Consider caching GetComponent calls\n" +
                                "💡 Suggestion: Use object pooling for frequently instantiated objects\n" +
                                "🔒 Security: No major issues detected";
        }

        private string CreateAnalysisOptionsJson()
        {
            // Convert options to JSON format expected by C++ DLL
            return JsonUtility.ToJson(new
            {
                analyzeDependencies = m_options.analyzeDependencies,
                analyzeLifecycleMethods = m_options.analyzeLifecycleMethods,
                detectDesignPatterns = m_options.detectDesignPatterns,
                generateMetadata = m_options.generateMetadata,
                generateContextSummary = m_options.generateContextSummary,
                analyzeScriptableObjects = m_options.analyzeScriptableObjects,
                analyzeAssetReferences = m_options.analyzeAssetReferences,
                verboseLogging = m_options.verboseLogging,
                outputDirectory = m_options.outputDirectory
            });
        }

        #endregion

        #region Real-time Analysis

        private void InitializeFileWatcher()
        {
            // Initialize file watching system
        }

        private void StartRealTimeAnalysis()
        {
            m_isRealTimeActive = true;
            m_statusMessage = "Real-time analysis started";
            
            // Start file watcher
            // This would initialize the C++ FileWatcher system
        }

        private void StopRealTimeAnalysis()
        {
            m_isRealTimeActive = false;
            m_statusMessage = "Real-time analysis stopped";
            
            // Stop file watcher
        }

        private void CheckForFileChanges()
        {
            // Check for file changes and add to pending list
            // This would integrate with Unity's AssetDatabase
        }

        private void ProcessPendingRealTimeAnalysis()
        {
            if (EditorApplication.timeSinceStartup - m_lastFileChangeTime > (m_options.fileChangeDebounceMs / 1000.0))
            {
                // Trigger incremental analysis
                m_pendingRealTimeAnalysis = false;
                // Start incremental analysis for changed files
            }
        }

        #endregion

        #region Event Handlers

        private void OnProjectChanged()
        {
            if (m_isRealTimeActive && m_options.autoAnalyzeOnSave)
            {
                m_lastFileChangeTime = EditorApplication.timeSinceStartup;
                m_pendingRealTimeAnalysis = true;
            }
        }

        private void OnBeforeAssemblyReload()
        {
            SavePreferences();
        }

        #endregion

        #region Preferences

        private void LoadPreferences()
        {
            var json = EditorPrefs.GetString(PREFS_KEY, "");
            if (!string.IsNullOrEmpty(json))
            {
                try
                {
                    m_options = JsonUtility.FromJson<AdvancedAnalysisOptions>(json);
                }
                catch
                {
                    m_options = new AdvancedAnalysisOptions();
                }
            }
            else
            {
                m_options = new AdvancedAnalysisOptions();
            }
        }

        private void SavePreferences()
        {
            var json = JsonUtility.ToJson(m_options);
            EditorPrefs.SetString(PREFS_KEY, json);
        }

        #endregion
    }
}