using System;
using System.Collections.Generic;
using UnityEngine;

namespace Unity.LLMContextGenerator
{
    /// <summary>
    /// Options for controlling the analysis process
    /// </summary>
    [Serializable]
    public class AnalysisOptions
    {
        [Header("Analysis Settings")]
        public bool analyzeDependencies = true;
        public bool analyzeLifecycleMethods = true;
        public bool detectDesignPatterns = true;
        public bool generateMetadata = true;
        public bool generateContextSummary = true;

        [Header("Output Settings")]
        public bool exportJson = true;
        public bool exportMarkdown = true;
        public bool exportLLMPrompt = true;
        public bool verboseLogging = false;

        [Header("Filter Settings")]
        public string[] includeNamespaces = new string[0];
        public string[] excludeNamespaces = new string[0];
        public string[] includeFilePatterns = new string[] { "*.cs" };
        public string[] excludeFilePatterns = new string[0];
    }

    /// <summary>
    /// Result of the analysis operation
    /// </summary>
    [Serializable]
    public class AnalysisResult
    {
        [Header("Status")]
        public bool Success;
        public string ErrorMessage;
        public float AnalysisDurationMs;
        
        [Header("Project Info")]
        public string ProjectType;
        public string Architecture;
        public int QualityScore;
        
        [Header("Component Analysis")]
        public int MonoBehaviourCount;
        public int SystemGroupCount;
        public int DependencyCount;
        public int DetectedPatternCount;
        
        [Header("Metadata")]
        public ComponentInfo[] Components;
        public DependencyInfo[] Dependencies;
        public PatternInfo[] DetectedPatterns;
        
        [Header("LLM Context")]
        public string ProjectContext;
        public string DevelopmentGuidelines;
        public string ArchitectureOverview;
        public string LLMPrompt;
    }

    /// <summary>
    /// Information about a Unity component
    /// </summary>
    [Serializable]
    public class ComponentInfo
    {
        public string Name;
        public string FilePath;
        public string BaseClass;
        public string Purpose;
        public string[] UnityMethods;
        public string[] CustomMethods;
        public string[] SerializedFields;
        public string[] Dependencies;
        public int ComplexityScore;
        public int StartLine;
        public int EndLine;
    }

    /// <summary>
    /// Information about component dependencies
    /// </summary>
    [Serializable]
    public class DependencyInfo
    {
        public string SourceComponent;
        public string TargetComponent;
        public string DependencyType;
        public string Context;
        public int LineNumber;
    }

    /// <summary>
    /// Information about detected design patterns
    /// </summary>
    [Serializable]
    public class PatternInfo
    {
        public string PatternName;
        public string Description;
        public string[] InvolvedComponents;
        public float ConfidenceScore;
        public string Evidence;
    }

    /// <summary>
    /// Quality metrics for the analyzed project
    /// </summary>
    [Serializable]
    public class QualityMetrics
    {
        public float MaintainabilityScore;
        public float TestabilityScore;
        public float PerformanceScore;
        public float ArchitectureScore;
        public string[] ImprovementSuggestions;
    }

    /// <summary>
    /// Event args for analysis progress updates
    /// </summary>
    public class AnalysisProgressEventArgs : EventArgs
    {
        public string CurrentFile { get; set; }
        public int FilesProcessed { get; set; }
        public int TotalFiles { get; set; }
        public string Status { get; set; }
        
        public float ProgressPercentage => TotalFiles > 0 ? (float)FilesProcessed / TotalFiles * 100f : 0f;
    }

    /// <summary>
    /// Event args for analysis completion
    /// </summary>
    public class AnalysisCompletedEventArgs : EventArgs
    {
        public AnalysisResult Result { get; set; }
        public TimeSpan Duration { get; set; }
        public bool Success { get; set; }
        public string ErrorMessage { get; set; }
    }
}