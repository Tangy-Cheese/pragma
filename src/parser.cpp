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

}


std::unique_ptr<return_node> Parser::returnparse(){

}

