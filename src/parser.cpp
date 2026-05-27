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
        expect(TokenType::L_PAREN);

        starting_node->param_ptrs = paramparse();

        expect(TokenType::R_PAREN);
        expect(TokenType::L_CURLY);

        starting_node->block_ptr = blockparse();
        advance();
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

        param_ptrs.push_back(std::move(p_node));
        advance();
    }
    return param_ptrs;
}


std::unique_ptr<block_node> Parser::blockparse(){
    
    // Add the pointers to the block_node: one return ptr and vector of astptrs.
    
    std::unique_ptr<block_node> block_ptr = std::make_unique<block_node>();
    std::vector<std::unique_ptr<AST_node>> body_nodes;

    while (peek().token_type != TokenType::R_CURLY){
        
        // For loop parser?, conditional parser? curerntly empty.
        
        if (peek().token_type == TokenType::RETURN){
            block_ptr->return_ptr = returnparse(); //Should push forward to return body
        }
    }

    block_ptr->body_ptrs = std::move(body_nodes);

    return block_ptr;
}


std::unique_ptr<return_node> Parser::returnparse(){

    std::unique_ptr<return_node> return_ptr = std::make_unique<return_node>();
    
    expect(TokenType::RETURN);
    return_ptr->expr_ptr = parse_expr();

    return return_ptr;
}


std::unique_ptr<AST_node> Parser::parse_expr(){

    std::unique_ptr<AST_node> expr_ptr = std::make_unique<AST_node>();
    
    while (peek().token_type != TokenType::L_CURLY){
        
    }
    return expr_ptr;
}


std::unique_ptr<AST_node> Parser::parse_primary(){

}


std::unique_ptr<AST_node> Parser::parse_expr(){


}
