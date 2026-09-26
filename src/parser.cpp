#include "parser.hpp"
#include "lexer.hpp"
#include "tokenizer.hpp"

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

Parser::Parser(std::vector<Token> token_vector){
    tokens = token_vector;
    curr_element = 0;
}


Parser::~Parser(){}


Token Parser::peek(){
    return tokens[curr_element];
}


void Parser::advance(){
    curr_element++;
}


void Parser::expect(TokenType expected){
    
    if (tokens[curr_element].token_type == expected){
        curr_element++;
    }
    else{
        std::cerr << "Wrong syntax: row" + std::to_string(tokens[curr_element].row_idx) << std::endl;
    }
}

    
std::unique_ptr<func_node> Parser::funcparse(){
    
    expect(TokenType::FUNC);
    Token identifier = peek();

    std::unique_ptr<func_node> starting_node = std::make_unique<func_node>();
    
    if (identifier.token_type == TokenType::IDENTIFIER){
        
        starting_node->token = identifier;

        advance();
        expect(TokenType::COLON);

        //need syntax checker here
        
        starting_node->return_type = peek().token_type;
        advance();

        expect(TokenType::L_PAREN);

        starting_node->param_ptrs = paramparse();

        expect(TokenType::R_PAREN);
        expect(TokenType::L_CURLY);

        starting_node->block_ptr = blockparse();
        expect(TokenType::R_CURLY);
    }

    else{
        std::cerr << "Missing function name: row" + std::to_string(identifier.row_idx) << std::endl;
    }

    return starting_node;
}


std::vector<std::unique_ptr<param_node>> Parser::paramparse(){

    // parses a list until right parenthesis
    std::vector<std::unique_ptr<param_node>> param_ptrs;

    while (peek().token_type != TokenType::R_PAREN){

        if (peek().token_type == TokenType::COMMA){
            advance();
        }

        std::unique_ptr<param_node> p_node = std::make_unique<param_node>();

        p_node->token = peek();

        advance();
        expect(TokenType::COLON);

        p_node->param_type = peek().token_type;
        advance();

        param_ptrs.push_back(std::move(p_node));
    }
    return param_ptrs;
}


std::unique_ptr<block_node> Parser::blockparse(){
    
    // Add the pointers to the block_node: one return ptr and vector of astptrs.
    
    std::unique_ptr<block_node> block_ptr = std::make_unique<block_node>();
    std::vector<std::unique_ptr<AST_node>> body_nodes;

    while (peek().token_type != TokenType::R_CURLY){

        // conditional parser? curerntly empty.

        if (peek().token_type == TokenType::RETURN){
            block_ptr->return_ptr = returnparse(); //Should push forward to return body
        }
        else if (peek().token_type == TokenType::IDENTIFIER){
            body_nodes.push_back(assignparse());
        }
        else if (peek().token_type == TokenType::FOR){
            body_nodes.push_back(forparse());
        }
        else{
            std::cerr << "Unsupported statement: row " + std::to_string(peek().row_idx) << std::endl;
            advance();
        }
    }

    block_ptr->body_ptrs = std::move(body_nodes);

    return block_ptr;
}


std::unique_ptr<return_node> Parser::returnparse(){

    std::unique_ptr<return_node> return_ptr = std::make_unique<return_node>();
    
    expect(TokenType::RETURN);
    return_ptr->expr_ptr = parse_expr(0);

    return return_ptr;
}


std::unique_ptr<assign_node> Parser::assignparse(){

    std::unique_ptr<assign_node> assign_ptr = std::make_unique<assign_node>();

    assign_ptr->token = peek();
    advance();

    Token op_token = peek();

    if (op_token.token_type != TokenType::EQUALS && op_token.token_type != TokenType::PLUSEQ &&
        op_token.token_type != TokenType::MINUSEQ && op_token.token_type != TokenType::ASTERISKEQ &&
        op_token.token_type != TokenType::SLASHEQ){
        std::cerr << "Expected assignment operator: row " + std::to_string(op_token.row_idx) << std::endl;
    }

    assign_ptr->op = op_token.token_type;
    advance();

    assign_ptr->expr_ptr = parse_expr(0);

    return assign_ptr;
}


std::unique_ptr<for_node> Parser::forparse(){

    std::unique_ptr<for_node> for_ptr = std::make_unique<for_node>();

    expect(TokenType::FOR);

    for_ptr->token = peek();
    advance();

    expect(TokenType::IN);

    for_ptr->range_start_ptr = parse_expr(0);
    expect(TokenType::RANGE);
    for_ptr->range_end_ptr = parse_expr(0);

    expect(TokenType::L_CURLY);

    std::vector<std::unique_ptr<AST_node>> body_nodes;

    while (peek().token_type != TokenType::R_CURLY){

        if (peek().token_type == TokenType::IDENTIFIER){
            body_nodes.push_back(assignparse());
        }
        else if (peek().token_type == TokenType::FOR){
            body_nodes.push_back(forparse());
        }
        else{
            std::cerr << "Unsupported statement: row " + std::to_string(peek().row_idx) << std::endl;
            advance();
        }
    }

    for_ptr->body_ptrs = std::move(body_nodes);

    expect(TokenType::R_CURLY);

    return for_ptr;
}


std::unique_ptr<AST_node> Parser::parse_primary(){
    Token t = peek();

    if (t.token_type == TokenType::IDENTIFIER || t.token_type == TokenType::INT_LIT || t.token_type == TokenType::FLOAT_LIT){

        auto node = std::make_unique<var_node>();
        node->token = t;
        advance();
        return node;
    }

    std::cerr << "Unexpected token in expression: row " << t.row_idx << std::endl;
    return nullptr;
}


std::unique_ptr<AST_node> Parser::parse_expr(int min_prec){

    auto left = parse_primary();

    while (op_prec(peek().token_type) > min_prec){

        Token op = peek();
        advance();
        auto right = parse_expr(op_prec(op.token_type));

        auto bin = std::make_unique<bin_expr_node>();
        bin->token = op;
        bin->operand = op.token_type;
        bin->var1_ptr = std::move(left);
        bin->var2_ptr = std::move(right);
        left = std::move(bin);
    }

    return left;
}


int Parser::op_prec(TokenType op_type){
    switch (op_type) { 
        default:
            return -1;
        case TokenType::PLUS: 
            return 10;
        case TokenType::MINUS: 
            return 10;
        case TokenType::SLASH: 
            return 20;
        case TokenType::ASTERISK: 
            return 20;
    }
}
