#pragma once

#include "parser.hpp"
#include "typecheck.hpp"
#include <map>
#include <memory>
#include <string>

class CodeGenerator{

public:
    CodeGenerator();
    ~CodeGenerator();

    /**
     * @brief Generates C code for a function node and writes it to output.c.
     */
    void generate_func(func_node*);

    std::unique_ptr<func_node> starting_node;

private:

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

public:

    // Mappings
    std::map<TokenType, std::string> type_string;
    std::map<TokenType, std::string> operator_string;

};
