#pragma once

#include "parser.hpp"
#include <fstream>
#include <map>
#include <memory>
#include <string>

class CodeGenerator{

public:
    CodeGenerator();
    ~CodeGenerator();

    /**
     * @brief Runs generate_func on starting_node.
     */
    void generate();

    std::unique_ptr<func_node> starting_node;

    // Mappings
    std::map<TokenType, std::string> type_string;
    std::map<TokenType, std::string> operator_string;

private:

    // Output file stream
    std::ofstream out_stream;

    /**
     * @brief Generates C code for a function node and writes it to out_stream.
     */
    void generate_func(func_node*);

    /**
     * @brief Generates C code for a block node's statements.
     */
    void generate_block(block_node*);

    /**
     * @brief Generates a C return statement from a return_node.
     */
    void generate_return(return_node*);

    /**
     * @brief Generates a C expression string from an AST_node.
     */
    std::string gen_expr(AST_node*);
};
