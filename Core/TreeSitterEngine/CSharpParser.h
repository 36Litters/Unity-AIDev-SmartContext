#pragma once

#include "TreeSitterWrapper.h"
#include "ASTNode.h"
#include <string>
#include <vector>
#include <memory>

namespace UnityContextGen {
namespace TreeSitter {

struct ClassInfo {
    std::string name;
    std::string base_class;
    std::vector<std::string> interfaces;
    std::vector<std::string> attributes;
    std::vector<std::string> methods;
    std::vector<std::string> fields;
    std::vector<std::string> properties;
    size_t start_line;
    size_t end_line;
};

struct MethodInfo {
    std::string name;
    std::string return_type;
    std::vector<std::string> parameters;
    std::vector<std::string> attributes;
    std::string access_modifier;
    bool is_static;
    bool is_virtual;
    bool is_override;
    size_t start_line;
    size_t end_line;
};

struct FieldInfo {
    std::string name;
    std::string type;
    std::vector<std::string> attributes;
    std::string access_modifier;
    bool is_static;
    bool is_readonly;
    size_t line;
};

class CSharpParser {
public:
    CSharpParser();
    ~CSharpParser() = default;

    bool parseFile(const std::string& file_path);
    bool parseCode(const std::string& source_code);
    
    std::vector<ClassInfo> getClasses() const;
    std::vector<MethodInfo> getMethods() const;
    std::vector<FieldInfo> getFields() const;
    
    std::vector<std::string> getUsingDirectives() const;
    std::string getNamespace() const;
    
    bool hasParseErrors() const;
    std::vector<std::string> getParseErrors() const;
    std::string getSourceCode() const;

private:
    TreeSitterWrapper m_wrapper;
    
    void analyzeNode(const std::unique_ptr<ASTNode>& node);
    void analyzeClass(const std::unique_ptr<ASTNode>& class_node);
    void analyzeMethod(const std::unique_ptr<ASTNode>& method_node);
    void analyzeField(const std::unique_ptr<ASTNode>& field_node);
    
    ClassInfo extractClassInfo(const std::unique_ptr<ASTNode>& class_node);
    MethodInfo extractMethodInfo(const std::unique_ptr<ASTNode>& method_node);
    FieldInfo extractFieldInfo(const std::unique_ptr<ASTNode>& field_node);
    
    std::vector<std::string> extractAttributes(const std::unique_ptr<ASTNode>& node);
    std::string extractAccessModifier(const std::unique_ptr<ASTNode>& node);
    std::vector<std::string> extractParameters(const std::unique_ptr<ASTNode>& params_node);
    
    std::vector<ClassInfo> m_classes;
    std::vector<MethodInfo> m_methods;
    std::vector<FieldInfo> m_fields;
    std::vector<std::string> m_using_directives;
    std::string m_namespace;
};

} // namespace TreeSitter
} // namespace UnityContextGen