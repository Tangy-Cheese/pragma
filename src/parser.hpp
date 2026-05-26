#pragma once

#include "lexer.hpp"
#include "tokenizer.hpp"
#include <memory>
#include <string>
#include <vector>

// forward decs
struct return_node;
struct param_node;
struct block_node;
struct bin_expr_node;
struct var_node;

class Parser{
public:
    
    Parser(std::vector<TokenType> token_vector);
    ~Parser();

    TokenType peek();
    void advance();
    void expect(TokenType expected);

    void funcparse();
    void blockparse();
    void returnparse();
    
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
};

struct param_node: public AST_node{
    std::string name;
};

struct block_node: public AST_node{
    std::unique_ptr<return_node> return_ptr;
};

struct return_node: public AST_node{
    std::unique_ptr<bin_expr_node> expr_ptr;
};

struct bin_expr_node: public AST_node{
    std::unique_ptr<AST_node> var1_ptr;
    std::unique_ptr<AST_node> var2_ptr;

    TokenType operand;
};

struct var_node: public AST_node{
    std::string name;
};


