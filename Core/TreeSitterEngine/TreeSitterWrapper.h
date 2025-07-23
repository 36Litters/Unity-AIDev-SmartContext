#pragma once

#include <tree_sitter/api.h>
#include <string>
#include <memory>
#include "ASTNode.h"

// Include the actual tree-sitter-c-sharp header
extern "C" {
    TSLanguage *tree_sitter_c_sharp(void);
}

namespace UnityContextGen {
namespace TreeSitter {

class TreeSitterWrapper {
public:
    TreeSitterWrapper();
    ~TreeSitterWrapper();

    bool parseCode(const std::string& source_code);
    std::unique_ptr<ASTNode> getRootNode() const;
    
    std::string getSourceCode() const { return m_source_code; }
    
    bool hasParseErrors() const;
    std::vector<std::string> getParseErrors() const;

private:
    TSParser* m_parser;
    TSTree* m_tree;
    std::string m_source_code;
    
    void cleanup();
    void collectParseErrors(TSNode node, std::vector<std::string>& errors) const;
};

} // namespace TreeSitter
} // namespace UnityContextGen