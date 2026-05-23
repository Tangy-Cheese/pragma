#pragma once

#include "tokenizer.hpp"
#include <cstddef>
#include <vector>
#include <string>
#include <fstream>

struct Group{

    TokenType token_type;
    std::string literal;
    size_t col_idx;
    size_t row_idx;
};

class Lexer{

public:
    Lexer();
    ~Lexer();
    
    /**
     * @brief Convert the .pragma file to a long string.
     */
    void conv_string(std::ifstream& input_file_stream);

    /**
     * @brief Function to scan the string and separate into tokens.
     *
     * @param fstring String which is being scanned.
     */
    void scan();

    /**
     * @brief Reading of the token vector.
     *
     * @param v Token vector.
     */
    std::vector<Group> get_token_vector();

private:
    std::vector<Group> v;

    std::string file_string;
    
    size_t index;
    size_t line_number;
};
