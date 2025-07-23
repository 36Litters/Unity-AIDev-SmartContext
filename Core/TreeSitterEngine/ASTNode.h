#pragma once

#include <string>
#include <vector>
#include <memory>
#include <tree_sitter/api.h>

namespace UnityContextGen {
namespace TreeSitter {

class ASTNode {
public:
    ASTNode(TSNode node, const std::string& source_code);
    ~ASTNode() = default;

    std::string getType() const;
    std::string getText() const;
    
    size_t getStartByte() const;
    size_t getEndByte() const;
    size_t getStartRow() const;
    size_t getStartColumn() const;
    size_t getEndRow() const;
    size_t getEndColumn() const;
    
    size_t getChildCount() const;
    std::unique_ptr<ASTNode> getChild(size_t index) const;
    std::vector<std::unique_ptr<ASTNode>> getChildren() const;
    
    std::unique_ptr<ASTNode> getChildByFieldName(const std::string& field_name) const;
    
    bool isNull() const;
    bool hasError() const;
    
    std::string getFieldName() const;
    
    std::vector<std::unique_ptr<ASTNode>> findNodesByType(const std::string& type) const;
    std::unique_ptr<ASTNode> findFirstNodeByType(const std::string& type) const;

private:
    TSNode m_node;
    const std::string& m_source_code;
    
    void findNodesByTypeRecursive(const std::string& type, 
                                  std::vector<std::unique_ptr<ASTNode>>& results) const;
};

} // namespace TreeSitter
} // namespace UnityContextGen