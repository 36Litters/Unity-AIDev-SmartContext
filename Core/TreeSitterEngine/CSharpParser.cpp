#include "CSharpParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace UnityContextGen {
namespace TreeSitter {

CSharpParser::CSharpParser() {
}

bool CSharpParser::parseFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source_code = buffer.str();
    
    return parseCode(source_code);
}

bool CSharpParser::parseCode(const std::string& source_code) {
    m_classes.clear();
    m_methods.clear();
    m_fields.clear();
    m_using_directives.clear();
    m_namespace.clear();
    
    if (!m_wrapper.parseCode(source_code)) {
        return false;
    }
    
    auto root = m_wrapper.getRootNode();
    if (!root) {
        return false;
    }
    
    analyzeNode(root);
    return true;
}

void CSharpParser::analyzeNode(const std::unique_ptr<ASTNode>& node) {
    if (!node || node->isNull()) {
        return;
    }
    
    std::string node_type = node->getType();
    
    // Debug output disabled for production
    
    if (node_type == "using_directive") {
        std::string using_text = node->getText();
        m_using_directives.push_back(using_text);
    }
    else if (node_type == "namespace_declaration") {
        auto name_node = node->getChildByFieldName("name");
        if (name_node) {
            m_namespace = name_node->getText();
        }
    }
    else if (node_type == "class_declaration") {
        analyzeClass(node);
    }
    else if (node_type == "method_declaration") {
        analyzeMethod(node);
    }
    else if (node_type == "field_declaration") {
        analyzeField(node);
    }
    
    auto children = node->getChildren();
    for (auto& child : children) {
        analyzeNode(child);
    }
}

void CSharpParser::analyzeClass(const std::unique_ptr<ASTNode>& class_node) {
    ClassInfo class_info = extractClassInfo(class_node);
    m_classes.push_back(class_info);
}

void CSharpParser::analyzeMethod(const std::unique_ptr<ASTNode>& method_node) {
    MethodInfo method_info = extractMethodInfo(method_node);
    m_methods.push_back(method_info);
}

void CSharpParser::analyzeField(const std::unique_ptr<ASTNode>& field_node) {
    FieldInfo field_info = extractFieldInfo(field_node);
    m_fields.push_back(field_info);
}

ClassInfo CSharpParser::extractClassInfo(const std::unique_ptr<ASTNode>& class_node) {
    ClassInfo info;
    
    // Debug output disabled for production
    
    auto name_node = class_node->getChildByFieldName("name");
    if (name_node) {
        info.name = name_node->getText();
    }
    
    // Find base_list by type instead of field name
    std::unique_ptr<ASTNode> base_list = nullptr;
    for (size_t i = 0; i < class_node->getChildCount(); ++i) {
        auto child = class_node->getChild(i);
        if (child && child->getType() == "base_list") {
            base_list = std::move(child);
            break;
        }
    }
    
    if (base_list) {
        auto children = base_list->getChildren();
        for (auto& child : children) {
            // The first non-colon child should be the base class
            if (child->getType() != ":" && child->getType() != ",") {
                if (info.base_class.empty()) {
                    info.base_class = child->getText();
                } else {
                    info.interfaces.push_back(child->getText());
                }
            }
        }
    }
    
    info.attributes = extractAttributes(class_node);
    info.start_line = class_node->getStartRow() + 1;
    info.end_line = class_node->getEndRow() + 1;
    
    return info;
}

MethodInfo CSharpParser::extractMethodInfo(const std::unique_ptr<ASTNode>& method_node) {
    MethodInfo info;
    
    auto name_node = method_node->getChildByFieldName("name");
    if (name_node) {
        info.name = name_node->getText();
    }
    
    auto type_node = method_node->getChildByFieldName("type");
    if (type_node) {
        info.return_type = type_node->getText();
    }
    
    auto params_node = method_node->getChildByFieldName("parameters");
    if (params_node) {
        info.parameters = extractParameters(params_node);
    }
    
    info.attributes = extractAttributes(method_node);
    info.access_modifier = extractAccessModifier(method_node);
    
    std::string method_text = method_node->getText();
    info.is_static = method_text.find("static") != std::string::npos;
    info.is_virtual = method_text.find("virtual") != std::string::npos;
    info.is_override = method_text.find("override") != std::string::npos;
    
    info.start_line = method_node->getStartRow() + 1;
    info.end_line = method_node->getEndRow() + 1;
    
    return info;
}

FieldInfo CSharpParser::extractFieldInfo(const std::unique_ptr<ASTNode>& field_node) {
    FieldInfo info;
    
    auto declaration = field_node->getChildByFieldName("declaration");
    if (declaration) {
        auto type_node = declaration->getChildByFieldName("type");
        if (type_node) {
            info.type = type_node->getText();
        }
        
        auto declarator = declaration->getChildByFieldName("declarator");
        if (declarator) {
            auto name_node = declarator->getChildByFieldName("name");
            if (name_node) {
                info.name = name_node->getText();
            }
        }
    }
    
    info.attributes = extractAttributes(field_node);
    info.access_modifier = extractAccessModifier(field_node);
    
    std::string field_text = field_node->getText();
    info.is_static = field_text.find("static") != std::string::npos;
    info.is_readonly = field_text.find("readonly") != std::string::npos;
    
    info.line = field_node->getStartRow() + 1;
    
    return info;
}

std::vector<std::string> CSharpParser::extractAttributes(const std::unique_ptr<ASTNode>& node) {
    std::vector<std::string> attributes;
    
    auto attr_lists = node->findNodesByType("attribute_list");
    for (auto& attr_list : attr_lists) {
        auto attrs = attr_list->findNodesByType("attribute");
        for (auto& attr : attrs) {
            attributes.push_back(attr->getText());
        }
    }
    
    return attributes;
}

std::string CSharpParser::extractAccessModifier(const std::unique_ptr<ASTNode>& node) {
    auto modifiers = node->findNodesByType("modifier");
    for (auto& modifier : modifiers) {
        std::string mod_text = modifier->getText();
        if (mod_text == "public" || mod_text == "private" || 
            mod_text == "protected" || mod_text == "internal") {
            return mod_text;
        }
    }
    return "private"; // default
}

std::vector<std::string> CSharpParser::extractParameters(const std::unique_ptr<ASTNode>& params_node) {
    std::vector<std::string> parameters;
    
    auto param_nodes = params_node->findNodesByType("parameter");
    for (auto& param : param_nodes) {
        parameters.push_back(param->getText());
    }
    
    return parameters;
}

std::vector<ClassInfo> CSharpParser::getClasses() const {
    return m_classes;
}

std::vector<MethodInfo> CSharpParser::getMethods() const {
    return m_methods;
}

std::vector<FieldInfo> CSharpParser::getFields() const {
    return m_fields;
}

std::vector<std::string> CSharpParser::getUsingDirectives() const {
    return m_using_directives;
}

std::string CSharpParser::getNamespace() const {
    return m_namespace;
}

bool CSharpParser::hasParseErrors() const {
    return m_wrapper.hasParseErrors();
}

std::vector<std::string> CSharpParser::getParseErrors() const {
    return m_wrapper.getParseErrors();
}

std::string CSharpParser::getSourceCode() const {
    return m_wrapper.getSourceCode();
}

} // namespace TreeSitter
} // namespace UnityContextGen