#include "../Core/TreeSitterEngine/CSharpParser.h"
#include "test_framework.h"
#include <iostream>
#include <fstream>

std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

void test_treesitter_engine() {
    using namespace UnityContextGen::TreeSitter;
    
    // Test 1: Parser initialization
    {
        CSharpParser parser;
        g_test_results.push_back({"CSharpParser_Initialize", true, "Parser initialized successfully"});
    }
    
    // Test 2: Parse simple C# code
    {
        CSharpParser parser;
        std::string simple_code = R"(
using UnityEngine;

public class TestClass : MonoBehaviour 
{
    void Start() 
    {
        Debug.Log("Hello World");
    }
}
)";
        
        bool success = parser.parseCode(simple_code);
        if (success) {
            auto classes = parser.getClasses();
            if (classes.size() == 1 && classes[0].name == "TestClass") {
                g_test_results.push_back({"CSharpParser_ParseSimpleCode", true, "Successfully parsed simple C# code"});
            } else {
                g_test_results.push_back({"CSharpParser_ParseSimpleCode", false, "Failed to extract class information correctly"});
            }
        } else {
            g_test_results.push_back({"CSharpParser_ParseSimpleCode", false, "Failed to parse simple C# code"});
        }
    }
    
    // Test 3: Parse PlayerController sample file
    {
        CSharpParser parser;
        std::string playerController = readFile("SampleUnityProject/PlayerController.cs");
        
        if (!playerController.empty()) {
            bool success = parser.parseCode(playerController);
            if (success) {
                auto classes = parser.getClasses();
                auto methods = parser.getMethods();
                
                bool foundPlayerController = false;
                bool foundAwakeMethod = false;
                bool foundUpdateMethod = false;
                
                for (const auto& cls : classes) {
                    if (cls.name == "PlayerController") {
                        foundPlayerController = true;
                        if (cls.base_class == "MonoBehaviour") {
                            // Good, it's a MonoBehaviour
                        }
                    }
                }
                
                for (const auto& method : methods) {
                    if (method.name == "Awake") foundAwakeMethod = true;
                    if (method.name == "Update") foundUpdateMethod = true;
                }
                
                if (foundPlayerController && foundAwakeMethod && foundUpdateMethod) {
                    g_test_results.push_back({"CSharpParser_ParsePlayerController", true, "Successfully parsed PlayerController with Unity methods"});
                } else {
                    g_test_results.push_back({"CSharpParser_ParsePlayerController", false, "Failed to find expected PlayerController structure"});
                }
            } else {
                g_test_results.push_back({"CSharpParser_ParsePlayerController", false, "Failed to parse PlayerController.cs"});
            }
        } else {
            g_test_results.push_back({"CSharpParser_ParsePlayerController", false, "Could not read PlayerController.cs file"});
        }
    }
    
    // Test 4: Extract using directives
    {
        CSharpParser parser;
        std::string code_with_usings = R"(
using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class TestClass : MonoBehaviour {}
)";
        
        bool success = parser.parseCode(code_with_usings);
        if (success) {
            auto usings = parser.getUsingDirectives();
            if (usings.size() >= 3) {
                g_test_results.push_back({"CSharpParser_ExtractUsings", true, "Successfully extracted using directives"});
            } else {
                g_test_results.push_back({"CSharpParser_ExtractUsings", false, "Failed to extract all using directives"});
            }
        } else {
            g_test_results.push_back({"CSharpParser_ExtractUsings", false, "Failed to parse code with using directives"});
        }
    }
    
    // Test 5: Parse code with attributes
    {
        CSharpParser parser;
        std::string code_with_attributes = R"(
using UnityEngine;

[RequireComponent(typeof(Rigidbody))]
public class TestClass : MonoBehaviour 
{
    [SerializeField]
    private float speed = 5.0f;
    
    [Header("Settings")]
    public int maxHealth = 100;
}
)";
        
        bool success = parser.parseCode(code_with_attributes);
        if (success) {
            auto classes = parser.getClasses();
            auto fields = parser.getFields();
            
            bool hasRequireComponent = false;
            bool hasSerializeField = false;
            
            for (const auto& cls : classes) {
                for (const auto& attr : cls.attributes) {
                    if (attr.find("RequireComponent") != std::string::npos) {
                        hasRequireComponent = true;
                    }
                }
            }
            
            for (const auto& field : fields) {
                for (const auto& attr : field.attributes) {
                    if (attr.find("SerializeField") != std::string::npos) {
                        hasSerializeField = true;
                    }
                }
            }
            
            if (hasRequireComponent || hasSerializeField) {
                g_test_results.push_back({"CSharpParser_ParseAttributes", true, "Successfully parsed attributes"});
            } else {
                g_test_results.push_back({"CSharpParser_ParseAttributes", false, "Failed to extract attributes"});
            }
        } else {
            g_test_results.push_back({"CSharpParser_ParseAttributes", false, "Failed to parse code with attributes"});
        }
    }
    
    // Test 6: Error handling
    {
        CSharpParser parser;
        std::string invalid_code = "this is not valid C# code {{{ ;;; )))";
        
        bool success = parser.parseCode(invalid_code);
        bool hasErrors = parser.hasParseErrors();
        
        if (!success || hasErrors) {
            g_test_results.push_back({"CSharpParser_ErrorHandling", true, "Properly handled invalid C# code"});
        } else {
            g_test_results.push_back({"CSharpParser_ErrorHandling", false, "Failed to detect invalid C# code"});
        }
    }
}