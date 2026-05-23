#pragma once

#include "tokenizer.hpp"
#include <cstddef>
#include <vector>
#include <string>
#include <fstream>

class Lexer{

public:
    Lexer();
    ~Lexer();
    
    void conv_string();
    void scan(std::string fstring);


    void get_token_vector(std::vector<std::string> v);

private:
    std::vector<std::string> v;
    std::string file_string;
    size_t index;

    size_t line_number;
};
