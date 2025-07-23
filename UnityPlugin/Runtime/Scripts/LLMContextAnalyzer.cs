using System;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Unity.LLMContextGenerator
{
    /// <summary>
    /// Main interface for Unity LLM Context Generator
    /// Provides P/Invoke interface to the native C++ analysis engine
    /// </summary>
    public static class LLMContextAnalyzer
    {
        #region Native Library Interface

        private const string NATIVE_LIBRARY = "unity_context_core";

        [DllImport(NATIVE_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr AnalyzeProject(string projectPath, string optionsJson);

        [DllImport(NATIVE_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr AnalyzeFile(string filePath, string optionsJson);

        [DllImport(NATIVE_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GenerateLLMPrompt(string analysisResultJson);

        [DllImport(NATIVE_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr GetLastError();

        [DllImport(NATIVE_LIBRARY, CallingConvention = CallingConvention.Cdecl)]
        private static extern void FreeString(IntPtr stringPtr);

        #endregion

        #region Public API

        /// <summary>
        /// Analyzes the current Unity project and generates LLM context
        /// </summary>
        /// <param name="options">Analysis options</param>
        /// <returns>Analysis result containing component metadata and LLM context</returns>
        public static AnalysisResult AnalyzeCurrentProject(AnalysisOptions options = null)
        {
            try
            {
                string projectPath = Application.dataPath;
                return AnalyzeProjectAtPath(projectPath, options);
            }
            catch (Exception e)
            {
                Debug.LogError($"[LLMContextGenerator] Failed to analyze current project: {e.Message}");
                return new AnalysisResult { Success = false, ErrorMessage = e.Message };
            }
        }

        /// <summary>
        /// Analyzes a Unity project at the specified path
        /// </summary>
        /// <param name="projectPath">Path to Unity project Assets folder</param>
        /// <param name="options">Analysis options</param>
        /// <returns>Analysis result</returns>
        public static AnalysisResult AnalyzeProjectAtPath(string projectPath, AnalysisOptions options = null)
        {
            try
            {
                if (string.IsNullOrEmpty(projectPath))
                {
                    return new AnalysisResult { Success = false, ErrorMessage = "Project path cannot be null or empty" };
                }

                options = options ?? new AnalysisOptions();
                string optionsJson = JsonUtility.ToJson(options);

                IntPtr resultPtr = AnalyzeProject(projectPath, optionsJson);
                string resultJson = MarshalPtrToString(resultPtr);
                
                if (string.IsNullOrEmpty(resultJson))
                {
                    string error = GetLastErrorString();
                    return new AnalysisResult { Success = false, ErrorMessage = error };
                }

                var result = JsonUtility.FromJson<AnalysisResult>(resultJson);
                return result ?? new AnalysisResult { Success = false, ErrorMessage = "Failed to parse analysis result" };
            }
            catch (Exception e)
            {
                Debug.LogError($"[LLMContextGenerator] Analysis failed: {e.Message}");
                return new AnalysisResult { Success = false, ErrorMessage = e.Message };
            }
        }

        /// <summary>
        /// Analyzes a single C# file
        /// </summary>
        /// <param name="filePath">Path to C# file</param>
        /// <param name="options">Analysis options</param>
        /// <returns>Analysis result for single file</returns>
        public static AnalysisResult AnalyzeSingleFile(string filePath, AnalysisOptions options = null)
        {
            try
            {
                if (string.IsNullOrEmpty(filePath))
                {
                    return new AnalysisResult { Success = false, ErrorMessage = "File path cannot be null or empty" };
                }

                options = options ?? new AnalysisOptions();
                string optionsJson = JsonUtility.ToJson(options);

                IntPtr resultPtr = AnalyzeFile(filePath, optionsJson);
                string resultJson = MarshalPtrToString(resultPtr);
                
                if (string.IsNullOrEmpty(resultJson))
                {
                    string error = GetLastErrorString();
                    return new AnalysisResult { Success = false, ErrorMessage = error };
                }

                var result = JsonUtility.FromJson<AnalysisResult>(resultJson);
                return result ?? new AnalysisResult { Success = false, ErrorMessage = "Failed to parse analysis result" };
            }
            catch (Exception e)
            {
                Debug.LogError($"[LLMContextGenerator] File analysis failed: {e.Message}");
                return new AnalysisResult { Success = false, ErrorMessage = e.Message };
            }
        }

        /// <summary>
        /// Generates an LLM-optimized prompt from analysis results
        /// </summary>
        /// <param name="analysisResult">Previous analysis result</param>
        /// <returns>LLM prompt text</returns>
        public static string GeneratePromptFromResult(AnalysisResult analysisResult)
        {
            try
            {
                if (analysisResult == null || !analysisResult.Success)
                {
                    return "# Error: Invalid or failed analysis result";
                }

                string analysisJson = JsonUtility.ToJson(analysisResult);
                IntPtr promptPtr = GenerateLLMPrompt(analysisJson);
                string prompt = MarshalPtrToString(promptPtr);
                
                return string.IsNullOrEmpty(prompt) ? "# Error: Failed to generate prompt" : prompt;
            }
            catch (Exception e)
            {
                Debug.LogError($"[LLMContextGenerator] Prompt generation failed: {e.Message}");
                return $"# Error: {e.Message}";
            }
        }

        #endregion

        #region Helper Methods

        private static string MarshalPtrToString(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                return null;

            try
            {
                string result = Marshal.PtrToStringAnsi(ptr);
                FreeString(ptr);
                return result;
            }
            catch (Exception)
            {
                return null;
            }
        }

        private static string GetLastErrorString()
        {
            try
            {
                IntPtr errorPtr = GetLastError();
                return MarshalPtrToString(errorPtr) ?? "Unknown error occurred";
            }
            catch
            {
                return "Failed to retrieve error message";
            }
        }

        #endregion
    }
}