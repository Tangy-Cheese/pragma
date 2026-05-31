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

    auto& param_vec = ptr->param_ptrs;

    //Fill env with param members
    for (auto& param: param_vec){
        env.insert({param->token.literal, param->param_type});
    }

    // TODO(human): walk ptr->block_ptr->return_ptr->expr_ptr, check_expr on it,
    //              compare result type against ptr->return_type
    
    auto& expression_pointer =  ptr->block_ptr->return_ptr->expr_ptr;
    std::optional<TokenType> expr_type = check_expr(expression_pointer.get());
    
    if (expr_type == std::nullopt){
        std::cerr << "This is just wrong" << std::endl;
        exit(1);
    }

    if (ptr->return_type != expr_type){
        std::cerr << "Type mismatch" << std::endl;
        exit(1);
    }
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
