#include "typecheck.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"
#include <iostream>
#include <memory>
#include <optional>

TypeChecker::TypeChecker(std::unique_ptr<func_node> function_node){
    root = std::move(function_node); 
}

TypeChecker::~TypeChecker(){}

void TypeChecker::checker(){    
    check_func(root.get());
}

void TypeChecker::check_func(func_node* ptr){
    check_expr(ptr);
}

std::optional<TokenType> TypeChecker::check_expr(AST_node* node){
    
    if(auto* leaf = dynamic_cast<var_node*>(node)){
        
        if (leaf->token.token_type == TokenType::IDENTIFIER){
            return env[leaf->token.literal];    
        }

        else{   
            switch (leaf->token.token_type){

                case (TokenType::INT_LIT):
                    return TokenType::INT_TYPE; 

                case (TokenType::FLOAT_LIT):
                    return TokenType::FLOAT_TYPE;   

                default:
                    std::cerr << "Bad return type" << std::endl;
            }
        }
    }

    if (auto* bin = dynamic_cast<bin_expr_node*>(node)){
        
        //Recurse here
        auto var1_branch = check_expr(bin->var1_ptr.get());
        auto var2_branch = check_expr(bin->var2_ptr.get());
        
        if (var1_branch == var2_branch){
            return var1_branch;
        }

        std::cerr << "Non matching types" << std::endl;
    }
    
    return std::nullopt;
}
