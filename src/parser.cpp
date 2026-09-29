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

    if (curr_element >= tokens.size()){
        size_t last_row = tokens.empty() ? 0 : tokens.back().row_idx;
        return {TokenType::END_OF_FILE, "", last_row};
    }
    return tokens[curr_element];
}


void Parser::advance(){
    curr_element++;
}


void Parser::expect(TokenType expected){
    
    if (peek().token_type == expected){
        curr_element++;
    }
    else{
        std::cerr << "Error: syntax error, unexpected token (row "
                  << peek().row_idx << ")." << std::endl;
    }
}


std::vector<std::unique_ptr<func_node>> Parser::programparse(){

    std::vector<std::unique_ptr<func_node>> funcs;
    std::vector<std::unique_ptr<AST_node>> top_level_stmts;
    size_t main_row = 0;

    while (curr_element < tokens.size()){

        if (peek().token_type == TokenType::FUNC){
            funcs.push_back(funcparse());
        }
        else{
            if (top_level_stmts.empty()) main_row = peek().row_idx;

            // stmtparse always consumes at least one token, so this can't loop forever.
            auto stmt = stmtparse();
            if (stmt) top_level_stmts.push_back(std::move(stmt));
        }
    }

    if (!top_level_stmts.empty()){

        // Top-level code becomes: int main(){ <statements> return 0; }
        main_func = std::make_unique<func_node>();
        main_func->token = {TokenType::IDENTIFIER, "main", main_row};
        main_func->return_type = TokenType::INT_TYPE;

        main_func->block_ptr = std::make_unique<block_node>();
        main_func->block_ptr->body_ptrs = std::move(top_level_stmts);

        auto zero = std::make_unique<var_node>();
        zero->token = {TokenType::INT_LIT, "0", main_row};

        main_func->block_ptr->return_ptr = std::make_unique<return_node>();
        main_func->block_ptr->return_ptr->expr_ptr = std::move(zero);
    }
    return funcs;
}


std::unique_ptr<func_node> Parser::take_main(){
    return std::move(main_func);
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
        std::cerr << "Error: missing function name (row " << identifier.row_idx << ")." << std::endl;
    }

    return starting_node;
}


std::vector<std::unique_ptr<param_node>> Parser::paramparse(){

    // parses a list until right parenthesis
    std::vector<std::unique_ptr<param_node>> param_ptrs;

    while (peek().token_type != TokenType::R_PAREN && peek().token_type != TokenType::END_OF_FILE){

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

    while (peek().token_type != TokenType::R_CURLY && peek().token_type != TokenType::END_OF_FILE){

        // conditional parser? curerntly empty.

        if (peek().token_type == TokenType::RETURN){
            block_ptr->return_ptr = returnparse(); //Should push forward to return body
        }
        else{
            auto stmt = stmtparse();
            if (stmt) body_nodes.push_back(std::move(stmt));
        }
    }

    block_ptr->body_ptrs = std::move(body_nodes);

    return block_ptr;
}


std::unique_ptr<AST_node> Parser::stmtparse(){

    if (peek().token_type == TokenType::IDENTIFIER){
        return assignparse();
    }
    else if (peek().token_type == TokenType::FOR){
        return forparse();
    }
    else if (peek().token_type == TokenType::PARALLEL){
        return parallelparse();
    }
    else if (peek().token_type == TokenType::PRINT){
        return printparse();
    }

    std::cerr << "Error: unsupported statement (row " << peek().row_idx << ")." << std::endl;
    advance();
    return nullptr;
}


std::unique_ptr<print_node> Parser::printparse(){

    std::unique_ptr<print_node> print_ptr = std::make_unique<print_node>();

    print_ptr->token = peek();
    expect(TokenType::PRINT);

    print_ptr->expr_ptr = parse_expr(0);

    return print_ptr;
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

    if (peek().token_type == TokenType::L_BRACKET){
        advance();
        assign_ptr->index_ptr = parse_expr(0);
        expect(TokenType::R_BRACKET);
    }

    Token op_token = peek();

    if (op_token.token_type != TokenType::EQUALS && op_token.token_type != TokenType::PLUSEQ &&
        op_token.token_type != TokenType::MINUSEQ && op_token.token_type != TokenType::ASTERISKEQ &&
        op_token.token_type != TokenType::SLASHEQ){
        std::cerr << "Error: expected an assignment operator (=, +=, -=, *=, or /=) (row "
                  << op_token.row_idx << ")." << std::endl;
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

    while (peek().token_type != TokenType::R_CURLY && peek().token_type != TokenType::END_OF_FILE){
        auto stmt = stmtparse();
        if (stmt) body_nodes.push_back(std::move(stmt));
    }

    for_ptr->body_ptrs = std::move(body_nodes);

    expect(TokenType::R_CURLY);

    return for_ptr;
}


std::unique_ptr<parallel_node> Parser::parallelparse(){

    std::unique_ptr<parallel_node> parallel_ptr = std::make_unique<parallel_node>();

    expect(TokenType::PARALLEL);

    parallel_ptr->token = peek();
    advance();

    expect(TokenType::IN);

    parallel_ptr->range_start_ptr = parse_expr(0);
    expect(TokenType::RANGE);
    parallel_ptr->range_end_ptr = parse_expr(0);

    expect(TokenType::L_CURLY);

    std::vector<std::unique_ptr<AST_node>> body_nodes;

    while (peek().token_type != TokenType::R_CURLY && peek().token_type != TokenType::END_OF_FILE){
        auto stmt = stmtparse();
        if (stmt) body_nodes.push_back(std::move(stmt));
    }

    parallel_ptr->body_ptrs = std::move(body_nodes);

    expect(TokenType::R_CURLY);

    return parallel_ptr;
}


std::unique_ptr<AST_node> Parser::parse_primary(){
    Token t = peek();

    if (t.token_type == TokenType::ALLOC){
        return allocparse();
    }

    if (t.token_type == TokenType::IDENTIFIER || t.token_type == TokenType::INT_LIT || t.token_type == TokenType::FLOAT_LIT){

        advance();

        if (t.token_type == TokenType::IDENTIFIER && peek().token_type == TokenType::L_PAREN){
            advance();

            auto call_ptr = std::make_unique<call_node>();
            call_ptr->token = t;

            if (peek().token_type != TokenType::R_PAREN){

                call_ptr->arg_ptrs.push_back(parse_expr(0));

                while (peek().token_type == TokenType::COMMA){
                    advance();
                    call_ptr->arg_ptrs.push_back(parse_expr(0));
                }
            }

            expect(TokenType::R_PAREN);

            return call_ptr;
        }

        if (t.token_type == TokenType::IDENTIFIER && peek().token_type == TokenType::L_BRACKET){
            advance();

            auto idx_node = std::make_unique<index_node>();
            idx_node->token = t;
            idx_node->index_ptr = parse_expr(0);

            expect(TokenType::R_BRACKET);

            return idx_node;
        }

        auto node = std::make_unique<var_node>();
        node->token = t;
        return node;
    }

    std::cerr << "Error: unexpected token in expression (row " << t.row_idx << ")." << std::endl;
    return nullptr;
}


std::unique_ptr<AST_node> Parser::allocparse(){

    expect(TokenType::ALLOC);
    expect(TokenType::L_PAREN);

    auto alloc_ptr = std::make_unique<alloc_node>();
    alloc_ptr->elem_type = peek().token_type;
    advance();

    expect(TokenType::COMMA);

    alloc_ptr->size_ptr = parse_expr(0);

    expect(TokenType::R_PAREN);

    return alloc_ptr;
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
        case TokenType::PERCENT:
            return 20;
    }
}
