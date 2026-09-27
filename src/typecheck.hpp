#pragma once

#include "parser.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

class TypeChecker{

public:

    TypeChecker(func_node*);
    ~TypeChecker();

    /**
     * @brief Runs check_func on member.
     */
    void checker();

    /**
     * @brief Read-only access to the type environment built up during checking.
     *
     * @return Map from variable name to inferred/declared type.
     */
    const std::unordered_map<std::string, TokenType>& get_env() const;

    /**
     * @brief Read-only access to the array element types built up during
     *  checking, keyed by array variable name.
     *
     * @return Map from array variable name to element type.
     */
    const std::unordered_map<std::string, TokenType>& get_array_env() const;

private:
    func_node* root;
    std::unordered_map<std::string, TokenType> env;
    std::unordered_map<std::string, TokenType> array_env;

    /**
     * @brief Type checks a function: binds param types into env, then checks
     *  the return expression against the declared return type.
     */
    void check_func(func_node*);

    /**
     * @brief Type checks a list of statements, growing env as locals are assigned.
     */
    void check_stmts(std::vector<std::unique_ptr<AST_node>>&);

    /**
     * @brief Type checks a for loop: checks its range bounds, binds the loop
     *  variable as int, then checks its body statements.
     */
    void check_for(for_node*);

    /**
     * @brief Recursively type checks an expression node.
     *
     * @return Resulting type of the expression, or nullopt on error.
     */
    std::optional<TokenType> check_expr(AST_node*);
};

