#include "TreeSitterWrapper.h"
#include <iostream>

namespace UnityContextGen {
namespace TreeSitter {

TreeSitterWrapper::TreeSitterWrapper() : m_parser(nullptr), m_tree(nullptr) {
    m_parser = ts_parser_new();
    
    if (!ts_parser_set_language(m_parser, tree_sitter_c_sharp())) {
        std::cerr << "Failed to set C# language for Tree-sitter parser" << std::endl;
        cleanup();
    }
}

TreeSitterWrapper::~TreeSitterWrapper() {
    cleanup();
}

void TreeSitterWrapper::cleanup() {
    if (m_tree) {
        ts_tree_delete(m_tree);
        m_tree = nullptr;
    }
    
    if (m_parser) {
        ts_parser_delete(m_parser);
        m_parser = nullptr;
    }
}

bool TreeSitterWrapper::parseCode(const std::string& source_code) {
    if (!m_parser) {
        return false;
    }
    
    if (m_tree) {
        ts_tree_delete(m_tree);
        m_tree = nullptr;
    }
    
    m_source_code = source_code;
    
    m_tree = ts_parser_parse_string(
        m_parser,
        nullptr,
        source_code.c_str(),
        source_code.length()
    );
    
    if (!m_tree) {
        std::cerr << "Failed to parse source code" << std::endl;
        return false;
    }
    
    TSNode root_node = ts_tree_root_node(m_tree);
    if (ts_node_has_error(root_node)) {
        std::cerr << "Parse tree contains errors" << std::endl;
    }
    
    return true;
}

std::unique_ptr<ASTNode> TreeSitterWrapper::getRootNode() const {
    if (!m_tree) {
        return nullptr;
    }
    
    TSNode root_node = ts_tree_root_node(m_tree);
    return std::make_unique<ASTNode>(root_node, m_source_code);
}

bool TreeSitterWrapper::hasParseErrors() const {
    if (!m_tree) {
        return true;
    }
    
    TSNode root_node = ts_tree_root_node(m_tree);
    return ts_node_has_error(root_node);
}

std::vector<std::string> TreeSitterWrapper::getParseErrors() const {
    std::vector<std::string> errors;
    
    if (!m_tree) {
        errors.push_back("No parse tree available");
        return errors;
    }
    
    TSNode root_node = ts_tree_root_node(m_tree);
    collectParseErrors(root_node, errors);
    
    return errors;
}

void TreeSitterWrapper::collectParseErrors(TSNode node, std::vector<std::string>& errors) const {
    if (ts_node_has_error(node)) {
        if (std::string(ts_node_type(node)) == "ERROR") {
            size_t start_row = ts_node_start_point(node).row + 1;
            size_t start_col = ts_node_start_point(node).column + 1;
            size_t end_row = ts_node_end_point(node).row + 1;
            size_t end_col = ts_node_end_point(node).column + 1;
            
            std::string error_msg = "Parse error at line " + std::to_string(start_row) +
                                   ", column " + std::to_string(start_col);
            
            if (start_row != end_row || start_col != end_col) {
                error_msg += " to line " + std::to_string(end_row) +
                            ", column " + std::to_string(end_col);
            }
            
            errors.push_back(error_msg);
        }
        
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            collectParseErrors(child, errors);
        }
    }
}

} // namespace TreeSitter
} // namespace UnityContextGen