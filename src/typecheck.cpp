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
        diagnostics.add(ptr->token.row_idx, "could not determine the type of the return expression in function '"
                         + ptr->token.literal + "'");
        return;
    }

    if (ptr->return_type != expr_type){
        diagnostics.add(ptr->token.row_idx, "function '" + ptr->token.literal + "' returns a value of the wrong type");
    }
}

const std::unordered_map<std::string, TokenType>& TypeChecker::get_env() const{
    return env;
}

const std::unordered_map<std::string, TokenType>& TypeChecker::get_array_env() const{
    return array_env;
}

const std::unordered_map<parallel_node*, std::vector<std::string>>& TypeChecker::get_parallel_clauses() const{
    return parallel_clauses;
}

const DiagnosticCollector& TypeChecker::get_diagnostics() const{
    return diagnostics;
}

void TypeChecker::check_stmts(std::vector<std::unique_ptr<AST_node>>& stmts){

    for (auto& stmt: stmts){

        if (auto* assign = dynamic_cast<assign_node*>(stmt.get())){

            if (assign->index_ptr){
                // Array element write: a[i] = expr. Doesn't touch env/array_env,
                // just checks the index and value against the array's element type.
                auto found = array_env.find(assign->token.literal);

                if (found == array_env.end()){
                    diagnostics.add(assign->token.row_idx, "'" + assign->token.literal + "' is not a declared array");
                    continue;
                }

                std::optional<TokenType> index_type = check_expr(assign->index_ptr.get());

                if (index_type != TokenType::INT_TYPE){
                    diagnostics.add(assign->token.row_idx, "array index for '" + assign->token.literal
                                     + "' must be of type int");
                    continue;
                }

                std::optional<TokenType> stmt_type = check_expr(assign->expr_ptr.get());

                if (stmt_type != found->second){
                    diagnostics.add(assign->token.row_idx, "value assigned to '" + assign->token.literal
                                     + "[...]' does not match its element type");
                    continue;
                }

                continue;
            }

            if (auto* alloc = dynamic_cast<alloc_node*>(assign->expr_ptr.get())){
                // Array declaration: a = alloc(type, size). Recorded in array_env,
                // never env, so a[i] and a are tracked as distinct namespaces.
                std::optional<TokenType> size_type = check_expr(alloc->size_ptr.get());

                if (size_type != TokenType::INT_TYPE){
                    diagnostics.add(assign->token.row_idx, "array size for '" + assign->token.literal
                                     + "' must be of type int");
                    continue;
                }

                array_env[assign->token.literal] = alloc->elem_type;
                continue;
            }

            std::optional<TokenType> stmt_type = check_expr(assign->expr_ptr.get());

            if (stmt_type == std::nullopt){
                diagnostics.add(assign->token.row_idx, "could not determine the type of the expression assigned to '"
                                 + assign->token.literal + "'");
                continue;
            }

            if (assign->op == TokenType::EQUALS){
                env[assign->token.literal] = stmt_type.value();
            }
            else{
                // Compound assignment (+=, -=, *=, /=) can't declare a new
                // local: there's no existing value to combine with.
                auto found = env.find(assign->token.literal);

                if (found == env.end()){
                    diagnostics.add(assign->token.row_idx, "'" + assign->token.literal
                                     + "' must be declared before it can be used with a compound assignment");
                    continue;
                }

                if (found->second != stmt_type){
                    diagnostics.add(assign->token.row_idx, "value assigned to '" + assign->token.literal
                                     + "' does not match its declared type");
                    continue;
                }
            }
        }

        else if (auto* for_stmt = dynamic_cast<for_node*>(stmt.get())){
            check_for(for_stmt);
        }

        else if (auto* parallel_stmt = dynamic_cast<parallel_node*>(stmt.get())){
            check_parallel(parallel_stmt);
        }
    }
}

void TypeChecker::check_for(for_node* ptr){

    std::optional<TokenType> start_type = check_expr(ptr->range_start_ptr.get());
    std::optional<TokenType> end_type = check_expr(ptr->range_end_ptr.get());

    if (start_type == std::nullopt || end_type == std::nullopt){
        diagnostics.add(ptr->token.row_idx, "could not determine the type of the loop range for '"
                         + ptr->token.literal + "'");
    }

    env[ptr->token.literal] = TokenType::INT_TYPE;

    check_stmts(ptr->body_ptrs);
}

void TypeChecker::check_parallel(parallel_node* ptr){

    std::optional<TokenType> start_type = check_expr(ptr->range_start_ptr.get());
    std::optional<TokenType> end_type = check_expr(ptr->range_end_ptr.get());

    if (start_type == std::nullopt || end_type == std::nullopt){
        diagnostics.add(ptr->token.row_idx, "could not determine the type of the parallel loop range for '"
                         + ptr->token.literal + "'");
    }

    env[ptr->token.literal] = TokenType::INT_TYPE;

    // OMP safety analysis: classify each top-level write in the body before
    // check_stmts mutates env below, since "already in env" is exactly how we
    // tell a shared outer variable from a fresh per-iteration local.
    std::vector<std::string> clauses;

    for (auto& stmt: ptr->body_ptrs){

        auto* assign = dynamic_cast<assign_node*>(stmt.get());
        if (!assign) continue;

        if (assign->index_ptr){
            // Array write: only a bare loop-variable index is provably a
            // unique write per iteration. Anything else can't be proven safe
            // yet (affine/nested-loop indices are a later pass).
            auto* idx_var = dynamic_cast<var_node*>(assign->index_ptr.get());
            bool unique_index = idx_var && idx_var->token.literal == ptr->token.literal;

            if (!unique_index){
                diagnostics.add(assign->token.row_idx, "array write index must be exactly the loop variable '"
                                 + ptr->token.literal + "' for this parallel loop to be provably safe");
            }
        }
        else{
            bool pre_existing = env.find(assign->token.literal) != env.end();

            if (pre_existing){
                if (assign->op == TokenType::PLUSEQ){
                    clauses.push_back("reduction(+:" + assign->token.literal + ")");
                }
                else if (assign->op == TokenType::ASTERISKEQ){
                    clauses.push_back("reduction(*:" + assign->token.literal + ")");
                }
                else{
                    diagnostics.add(assign->token.row_idx, "'" + assign->token.literal
                                     + "' is written without a reduction operator inside a parallel loop. "
                                       "Use += or *= to combine into it safely");
                }
            }
            // else: name is new here, becomes a loop-body-local declaration in
            // codegen, so each iteration gets its own storage - no clause needed.
        }
    }

    parallel_clauses[ptr] = clauses;

    check_stmts(ptr->body_ptrs);
}

std::optional<TokenType> TypeChecker::check_expr(AST_node* node){
    
    if(auto* leaf = dynamic_cast<var_node*>(node)){
        
        if (leaf->token.token_type == TokenType::IDENTIFIER){

            auto found = env.find(leaf->token.literal);

            if (found == env.end()){
                diagnostics.add(leaf->token.row_idx, "'" + leaf->token.literal + "' is not declared");
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
                    diagnostics.add(leaf->token.row_idx, "unsupported literal in expression");
            }
        }
    }

    if (auto* idx = dynamic_cast<index_node*>(node)){

        auto found = array_env.find(idx->token.literal);

        if (found == array_env.end()){
            diagnostics.add(idx->token.row_idx, "'" + idx->token.literal + "' is not a declared array");
            return std::nullopt;
        }

        std::optional<TokenType> index_type = check_expr(idx->index_ptr.get());

        if (index_type != TokenType::INT_TYPE){
            diagnostics.add(idx->token.row_idx, "array index for '" + idx->token.literal + "' must be of type int");
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

        diagnostics.add(bin->token.row_idx, "mismatched operand types around '" + bin->token.literal + "'");
    }
    
    return std::nullopt;
}
