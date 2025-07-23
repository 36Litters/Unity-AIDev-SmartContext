#include "ASTNode.h"
#include <cstring>

namespace UnityContextGen {
namespace TreeSitter {

ASTNode::ASTNode(TSNode node, const std::string& source_code) 
    : m_node(node), m_source_code(source_code) {
}

std::string ASTNode::getType() const {
    return std::string(ts_node_type(m_node));
}

std::string ASTNode::getText() const {
    size_t start = getStartByte();
    size_t end = getEndByte();
    
    if (start >= m_source_code.length() || end > m_source_code.length()) {
        return "";
    }
    
    return m_source_code.substr(start, end - start);
}

size_t ASTNode::getStartByte() const {
    return ts_node_start_byte(m_node);
}

size_t ASTNode::getEndByte() const {
    return ts_node_end_byte(m_node);
}

size_t ASTNode::getStartRow() const {
    return ts_node_start_point(m_node).row;
}

size_t ASTNode::getStartColumn() const {
    return ts_node_start_point(m_node).column;
}

size_t ASTNode::getEndRow() const {
    return ts_node_end_point(m_node).row;
}

size_t ASTNode::getEndColumn() const {
    return ts_node_end_point(m_node).column;
}

size_t ASTNode::getChildCount() const {
    return ts_node_child_count(m_node);
}

std::unique_ptr<ASTNode> ASTNode::getChild(size_t index) const {
    if (index >= getChildCount()) {
        return nullptr;
    }
    
    TSNode child = ts_node_child(m_node, index);
    if (ts_node_is_null(child)) {
        return nullptr;
    }
    
    return std::make_unique<ASTNode>(child, m_source_code);
}

std::vector<std::unique_ptr<ASTNode>> ASTNode::getChildren() const {
    std::vector<std::unique_ptr<ASTNode>> children;
    size_t count = getChildCount();
    
    for (size_t i = 0; i < count; ++i) {
        auto child = getChild(i);
        if (child) {
            children.push_back(std::move(child));
        }
    }
    
    return children;
}

std::unique_ptr<ASTNode> ASTNode::getChildByFieldName(const std::string& field_name) const {
    TSNode child = ts_node_child_by_field_name(m_node, field_name.c_str(), field_name.length());
    
    if (ts_node_is_null(child)) {
        return nullptr;
    }
    
    return std::make_unique<ASTNode>(child, m_source_code);
}

bool ASTNode::isNull() const {
    return ts_node_is_null(m_node);
}

bool ASTNode::hasError() const {
    return ts_node_has_error(m_node);
}

std::string ASTNode::getFieldName() const {
    const char* field_name = ts_node_field_name_for_child(m_node, 0);
    return field_name ? std::string(field_name) : "";
}

std::vector<std::unique_ptr<ASTNode>> ASTNode::findNodesByType(const std::string& type) const {
    std::vector<std::unique_ptr<ASTNode>> results;
    findNodesByTypeRecursive(type, results);
    return results;
}

std::unique_ptr<ASTNode> ASTNode::findFirstNodeByType(const std::string& type) const {
    if (getType() == type) {
        return std::make_unique<ASTNode>(m_node, m_source_code);
    }
    
    auto children = getChildren();
    for (auto& child : children) {
        auto result = child->findFirstNodeByType(type);
        if (result) {
            return result;
        }
    }
    
    return nullptr;
}

void ASTNode::findNodesByTypeRecursive(const std::string& type, 
                                       std::vector<std::unique_ptr<ASTNode>>& results) const {
    if (getType() == type) {
        results.push_back(std::make_unique<ASTNode>(m_node, m_source_code));
    }
    
    auto children = getChildren();
    for (auto& child : children) {
        child->findNodesByTypeRecursive(type, results);
    }
}

} // namespace TreeSitter
} // namespace UnityContextGen