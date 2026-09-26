#include "generator.hpp"
#include "lexer.hpp"
#include "tokenizer.hpp"

#include <fstream>
#include <iostream>

CodeGenerator::CodeGenerator(){
    out_stream.open("output.c");

    // building maps
    type_string.insert({TokenType::INT_TYPE, "int"});
    type_string.insert({TokenType::DOUBLE_TYPE, "double"});
    type_string.insert({TokenType::FLOAT_TYPE, "float"});
    type_string.insert({TokenType::BOOL_TYPE, "bool"});
    type_string.insert({TokenType::COMPLEX_TYPE, "complex"});

    operator_string.insert({TokenType::PLUS, "+"});
    operator_string.insert({TokenType::MINUS, "-"});
    operator_string.insert({TokenType::ASTERISK, "*"});
    operator_string.insert({TokenType::SLASH, "/"});
    operator_string.insert({TokenType::PERCENT, "%"});
    operator_string.insert({TokenType::EQUALS, "="});
    operator_string.insert({TokenType::PLUSEQ, "+="});
    operator_string.insert({TokenType::MINUSEQ, "-="});
    operator_string.insert({TokenType::ASTERISKEQ, "*="});
    operator_string.insert({TokenType::SLASHEQ, "/="});
    operator_string.insert({TokenType::GREATER, ">"});
    operator_string.insert({TokenType::LESS, "<"});
    operator_string.insert({TokenType::GREATEREQ, ">="});
    operator_string.insert({TokenType::LESSEQ, "<="});
    operator_string.insert({TokenType::NOTEQ, "!="});
    operator_string.insert({TokenType::AND, "&&"});
    operator_string.insert({TokenType::OR, "||"});
}

CodeGenerator::~CodeGenerator(){}


void CodeGenerator::generate(){
    generate_func(starting_node);
}


void CodeGenerator::generate_func(func_node* ptr){

    declared_vars.clear();

    for (auto& param: ptr->param_ptrs){
        declared_vars.insert(param->token.literal);
    }

    out_stream << type_string[ptr->return_type] << " " << ptr->token.literal << "(";

    for (size_t i=0; i<ptr->param_ptrs.size(); i++){

        if (i>0) out_stream << ", ";

        auto& param = ptr->param_ptrs[i];
        out_stream << type_string[param->param_type] << " " << param->token.literal;
    }

    out_stream << "){\n";

    generate_block(ptr->block_ptr.get());

    out_stream << "}\n";
}


void CodeGenerator::generate_block(block_node* ptr){

    // conditional parser? currently empty on parser side too.

    generate_stmts(ptr->body_ptrs);

    generate_return(ptr->return_ptr.get());
}


void CodeGenerator::generate_stmts(std::vector<std::unique_ptr<AST_node>>& stmts){

    for (auto& stmt: stmts){

        if (auto* assign = dynamic_cast<assign_node*>(stmt.get())){
            generate_assign(assign);
        }

        else if (auto* for_stmt = dynamic_cast<for_node*>(stmt.get())){
            generate_for(for_stmt);
        }
    }
}


void CodeGenerator::generate_assign(assign_node* ptr){

    std::string name = ptr->token.literal;

    if (declared_vars.count(name) == 0){
        // ptr->op is always EQUALS on a first declaration; typecheck rejects
        // a compound assignment to an undeclared variable before this runs.
        out_stream << "    " << type_string[env[name]] << " " << name << " = " << gen_expr(ptr->expr_ptr.get()) << ";\n";
        declared_vars.insert(name);
    }
    else{
        out_stream << "    " << name << " " << operator_string[ptr->op] << " " << gen_expr(ptr->expr_ptr.get()) << ";\n";
    }
}


void CodeGenerator::generate_for(for_node* ptr){

    std::string var = ptr->token.literal;

    out_stream << "    for (int " << var << " = " << gen_expr(ptr->range_start_ptr.get())
               << "; " << var << " < " << gen_expr(ptr->range_end_ptr.get())
               << "; " << var << "++){\n";

    generate_stmts(ptr->body_ptrs);

    out_stream << "    }\n";
}


void CodeGenerator::generate_return(return_node* ptr){
    out_stream << "    return " << gen_expr(ptr->expr_ptr.get()) << ";\n";
}


std::string CodeGenerator::gen_expr(AST_node* node){

    if (auto* leaf = dynamic_cast<var_node*>(node)){
        return leaf->token.literal;
    }

    if (auto* bin = dynamic_cast<bin_expr_node*>(node)){
        return "(" + gen_expr(bin->var1_ptr.get()) + " " + operator_string[bin->operand] + " " + gen_expr(bin->var2_ptr.get()) + ")";
    }

    std::cerr << "Unknown expression node" << std::endl;
    return "";
}
