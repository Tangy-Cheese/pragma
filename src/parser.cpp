#include "parser.hpp"
#include "lexer.hpp"
#include "tokenizer.hpp"
#include <vector>

Parser::Parser(std::vector<TokenType> token_vector){
    tokens = token_vector;
}

Parser::~Parser(){}

TokenType Parser::peek(){

}

void Parser::advance(){
}

void Parser::expect(TokenType expected){
    
}
    
void Parser::funcparse(){

}

void Parser::blockparse(){

}

void Parser::returnparse(){

}

