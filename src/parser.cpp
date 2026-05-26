#include "parser.hpp"
#include "lexer.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <ostream>
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
        std::cerr << "Wrong syntax" << std::endl;
    }
}
    
void Parser::funcparse(){
    
    expect(TokenType::FUNC);

}

void Parser::blockparse(){

}

void Parser::returnparse(){

}

