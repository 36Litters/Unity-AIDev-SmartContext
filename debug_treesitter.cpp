#include <iostream>
#include <cstring>
#include <tree_sitter/api.h>

extern "C" {
    TSLanguage *tree_sitter_c_sharp(void);
}

int main() {
    // Test if tree_sitter_c_sharp works
    TSParser *parser = ts_parser_new();
    TSLanguage *language = tree_sitter_c_sharp();
    
    if (language == nullptr) {
        std::cout << "ERROR: tree_sitter_c_sharp returned null" << std::endl;
        return 1;
    }
    
    bool success = ts_parser_set_language(parser, language);
    if (!success) {
        std::cout << "ERROR: Failed to set C# language" << std::endl;
        return 1;
    }
    
    // Simple C# code to test
    const char* source_code = "public class Test : MonoBehaviour { void Start() { } }";
    
    TSTree *tree = ts_parser_parse_string(parser, nullptr, source_code, strlen(source_code));
    if (tree == nullptr) {
        std::cout << "ERROR: Failed to parse code" << std::endl;
        return 1;
    }
    
    TSNode root_node = ts_tree_root_node(tree);
    char *string = ts_node_string(root_node);
    std::cout << "Parse tree:" << std::endl;
    std::cout << string << std::endl;
    
    free(string);
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    
    std::cout << "SUCCESS: Tree-sitter C# is working!" << std::endl;
    return 0;
}