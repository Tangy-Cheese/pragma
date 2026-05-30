#pragma once

#include "lexer.hpp"
#include "tokenizer.hpp"

#include <memory>
#include <vector>

// forward decs
struct AST_node;
struct func_node;
struct return_node;
struct param_node;
struct block_node;
struct bin_expr_node;
struct var_node;

class Parser{
public:
    
    Parser(std::vector<Token> token_vector);
    ~Parser();

    /**
     * @brief Returns whatever the current element is pointing at.
     *
     * @return Token of the current element.
     */
    Token peek();

    /**
     * @brief Adnaces the curr_element counter.
     */
    void advance();
    
    /**
     * @brief Checking for expected syntax: braces, paranthesis etc.
     *
     * @param expected TokenType we expect from the curr_element.
     */
    void expect(TokenType expected);

    /**
     * @brief Returns the pointer to the starting node of the functional abstract syntax tree.
     */
    std::unique_ptr<func_node> funcparse();

    /**
     * @brief Returns a vector of pointers to param nodes. Remember they are ending nodes so having them
     *  as a vector is fine.
     */
    std::vector<std::unique_ptr<param_node>> paramparse();

    /**
     * @brief 
     */
    std::unique_ptr<block_node> blockparse();

    /**
     * @brief 
     */
    std::unique_ptr<return_node> returnparse();

    /**
     * @brief 
     *
     * @param min_prec 
     */
    std::unique_ptr<AST_node> parse_expr(int min_prec);

    /**
     * @brief 
     */
    std::unique_ptr<AST_node> parse_primary();

    /**
     * @brief 
     *
     * @param op_type 
     * @return 
     */
    int op_prec(TokenType op_type);

private:
    size_t curr_element;
    std::vector<Token> tokens;
};

struct AST_node{
    Token token;
    virtual ~AST_node()=default;
};

struct func_node: public AST_node{
    std::vector<std::unique_ptr<param_node>> param_ptrs;
    std::unique_ptr<block_node> block_ptr;

    TokenType return_type;
};

struct param_node: public AST_node{
    TokenType param_type;
};

struct block_node: public AST_node{
    std::unique_ptr<return_node> return_ptr;
    std::vector<std::unique_ptr<AST_node>> body_ptrs;
};

struct return_node: public AST_node{
    std::unique_ptr<AST_node> expr_ptr;
};

struct bin_expr_node: public AST_node{
    std::unique_ptr<AST_node> var1_ptr;
    std::unique_ptr<AST_node> var2_ptr;

    TokenType operand;
};

struct var_node: public AST_node{
};

