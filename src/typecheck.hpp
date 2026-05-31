#pragma once

#include "parser.hpp"
#include <memory>
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

private:
    std::unique_ptr<func_node> root;
    std::unordered_map<std::string, TokenType> env;

    /**
     * @brief 
     */
    void check_func(func_node*);

    /**
     * @brief 
     *
     * @return 
     */
    std::optional<TokenType> check_expr(AST_node*);
};

