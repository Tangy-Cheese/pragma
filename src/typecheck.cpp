#include "typecheck.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"

#include <iostream>
#include <optional>

TypeChecker::TypeChecker(func_node* function_node){
    root = function_node;
}

TypeChecker::~TypeChecker(){}

void TypeChecker::checker(){
    check_func(root);
}

void TypeChecker::check_func(func_node* ptr){

    auto& param_vec = ptr->param_ptrs;

    //Fill env with param members
    for (auto& param: param_vec){
        env.insert({param->token.literal, param->param_type});
    }

    check_stmts(ptr->block_ptr->body_ptrs);

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

const std::unordered_map<std::string, TokenType>& TypeChecker::get_env() const{
    return env;
}

const std::unordered_map<std::string, TokenType>& TypeChecker::get_array_env() const{
    return array_env;
}

void TypeChecker::check_stmts(std::vector<std::unique_ptr<AST_node>>& stmts){

    for (auto& stmt: stmts){

        if (auto* assign = dynamic_cast<assign_node*>(stmt.get())){

            if (assign->index_ptr){
                // Array element write: a[i] = expr. Doesn't touch env/array_env,
                // just checks the index and value against the array's element type.
                auto found = array_env.find(assign->token.literal);

                if (found == array_env.end()){
                    std::cerr << "Undeclared array: " << assign->token.literal << std::endl;
                    exit(1);
                }

                std::optional<TokenType> index_type = check_expr(assign->index_ptr.get());

                if (index_type != TokenType::INT_TYPE){
                    std::cerr << "Array index must be int" << std::endl;
                    exit(1);
                }

                std::optional<TokenType> stmt_type = check_expr(assign->expr_ptr.get());

                if (stmt_type != found->second){
                    std::cerr << "Type mismatch" << std::endl;
                    exit(1);
                }

                continue;
            }

            if (auto* alloc = dynamic_cast<alloc_node*>(assign->expr_ptr.get())){
                // Array declaration: a = alloc(type, size). Recorded in array_env,
                // never env, so a[i] and a are tracked as distinct namespaces.
                std::optional<TokenType> size_type = check_expr(alloc->size_ptr.get());

                if (size_type != TokenType::INT_TYPE){
                    std::cerr << "Array size must be int" << std::endl;
                    exit(1);
                }

                array_env[assign->token.literal] = alloc->elem_type;
                continue;
            }

            std::optional<TokenType> stmt_type = check_expr(assign->expr_ptr.get());

            if (stmt_type == std::nullopt){
                std::cerr << "This is just wrong" << std::endl;
                exit(1);
            }

            if (assign->op == TokenType::EQUALS){
                env[assign->token.literal] = stmt_type.value();
            }
            else{
                // Compound assignment (+=, -=, *=, /=) can't declare a new
                // local: there's no existing value to combine with.
                auto found = env.find(assign->token.literal);

                if (found == env.end()){
                    std::cerr << "Undeclared identifier: " << assign->token.literal << std::endl;
                    exit(1);
                }

                if (found->second != stmt_type){
                    std::cerr << "Type mismatch" << std::endl;
                    exit(1);
                }
            }
        }

        else if (auto* for_stmt = dynamic_cast<for_node*>(stmt.get())){
            check_for(for_stmt);
        }
    }
}

void TypeChecker::check_for(for_node* ptr){

    std::optional<TokenType> start_type = check_expr(ptr->range_start_ptr.get());
    std::optional<TokenType> end_type = check_expr(ptr->range_end_ptr.get());

    if (start_type == std::nullopt || end_type == std::nullopt){
        std::cerr << "This is just wrong" << std::endl;
        exit(1);
    }

    env[ptr->token.literal] = TokenType::INT_TYPE;

    check_stmts(ptr->body_ptrs);
}

std::optional<TokenType> TypeChecker::check_expr(AST_node* node){
    
    if(auto* leaf = dynamic_cast<var_node*>(node)){
        
        if (leaf->token.token_type == TokenType::IDENTIFIER){

            auto found = env.find(leaf->token.literal);

            if (found == env.end()){
                std::cerr << "Undeclared identifier: " << leaf->token.literal << std::endl;
                return std::nullopt;
            }

            return found->second;
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

    if (auto* idx = dynamic_cast<index_node*>(node)){

        auto found = array_env.find(idx->token.literal);

        if (found == array_env.end()){
            std::cerr << "Undeclared array: " << idx->token.literal << std::endl;
            return std::nullopt;
        }

        std::optional<TokenType> index_type = check_expr(idx->index_ptr.get());

        if (index_type != TokenType::INT_TYPE){
            std::cerr << "Array index must be int" << std::endl;
            return std::nullopt;
        }

        return found->second;
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
