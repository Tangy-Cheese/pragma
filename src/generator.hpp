#pragma once

#include "parser.hpp"
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

struct func_unit{
    func_node* func;
    std::unordered_map<std::string, TokenType> env;
    std::unordered_map<parallel_node*, std::vector<std::string>> parallel_clauses;
};

class CodeGenerator{

public:
    CodeGenerator();
    ~CodeGenerator();

    /**
     * @brief Runs generate_func on starting_node.
     */
    void generate();

    /**
     * @brief Writes the includes once, then generates every function with
     *  its own env and parallel clauses.
     *
     * @param units One func_unit per function, in source order.
     */
    void generate_program(std::vector<func_unit>& units);

    func_node* starting_node;

    // Type environment from TypeChecker::get_env(), keyed by variable name
    std::unordered_map<std::string, TokenType> env;

    // OpenMP clauses from TypeChecker::get_parallel_clauses(), keyed by node pointer
    std::unordered_map<parallel_node*, std::vector<std::string>> parallel_clauses;

    // Mappings
    std::map<TokenType, std::string> type_string;
    std::map<TokenType, std::string> operator_string;

private:

    // Output file stream
    std::ofstream out_stream;

    // Local vars already declared in the current function
    std::set<std::string> declared_vars;

    /**
     * @brief Generates C code for a function node and writes it to out_stream.
     */
    void generate_func(func_node*);

    /**
     * @brief Generates C code for a block node's statements.
     */
    void generate_block(block_node*);

    /**
     * @brief Generates C code for a list of statements.
     */
    void generate_stmts(std::vector<std::unique_ptr<AST_node>>&);

    /**
     * @brief Generates a C assignment statement from an assign_node, declaring
     *  the variable's type on its first assignment.
     */
    void generate_assign(assign_node*);

    /**
     * @brief Generates a C for loop from a for_node.
     */
    void generate_for(for_node*);

    /**
     * @brief Generates a C for loop annotated with a bare `#pragma omp
     *  parallel for` from a parallel_node. Clause selection (reduction,
     *  collapse, etc.) is a separate later pass, not part of codegen yet.
     */
    void generate_parallel(parallel_node*);

    /**
     * @brief Generates a C return statement from a return_node.
     */
    void generate_return(return_node*);

    /**
     * @brief Generates a C expression string from an AST_node.
     */
    std::string gen_expr(AST_node*);
};
