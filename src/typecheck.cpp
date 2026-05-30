#include "typecheck.hpp"
#include "parser.hpp"
#include <memory>

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

TokenType TypeChecker::check_expr(AST_node* node){
    
    if(auto* leaf = dynamic_cast<var_node*>(node)){

        auto lit_type = leaf->token.token_type;
        e


        return ;
    }
    
    return TokenType::INT_TYPE; // placeholder for non-leaf nodes (handled later)
}
