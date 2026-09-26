#pragma once

#include "parser.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

class TypeChecker{

public:

    TypeChecker(std::unique_ptr<func_node>);
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

private:
    std::unique_ptr<func_node> root;
    std::unordered_map<std::string, TokenType> env;

    /**
     * @brief Type checks a function: binds param types into env, then checks
     *  the return expression against the declared return type.
     */
    void check_func(func_node*);

    /**
     * @brief Recursively type checks an expression node.
     *
     * @return Resulting type of the expression, or nullopt on error.
     */
    std::optional<TokenType> check_expr(AST_node*);
};

