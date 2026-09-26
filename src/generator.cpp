#include "generator.hpp"
#include "lexer.hpp"
#include "tokenizer.hpp"

#include <fstream>

CodeGenerator::CodeGenerator(){
    std::ofstream MyFile("output.c");




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
    operator_string.insert({TokenType::GREATER, "<"});
    operator_string.insert({TokenType::LESS, ">"});
    operator_string.insert({TokenType::GREATEREQ, "<="});
    operator_string.insert({TokenType::LESSEQ, ">="});
    operator_string.insert({TokenType::NOTEQ, "!="});
    operator_string.insert({TokenType::AND, "&&"});
    operator_string.insert({TokenType::OR, "||"});
}
