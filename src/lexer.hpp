#pragma once

#include "tokenizer.hpp"
#include <cstddef>
#include <iterator>
#include <vector>
#include <string>
#include <fstream>
#include <map>

struct Token{

    TokenType token_type;
    std::string literal;
    size_t row_idx;

    // Equals operator overload
    bool operator==(const Token& other) const{
        return token_type == other.token_type && literal == other.literal && row_idx == other.row_idx;
    }
};

class Lexer{

public:
    Lexer(std::string filename);
    ~Lexer();
    
    /**
     * @brief Convert the .pragma file to a vector of long strings indexed by row number.
     */
    void conv_string(std::ifstream& input_file_stream);

    /**
     * @brief 
     *
     * @param type
     * @param lit 
     * @param row 
     */
    void push_token(TokenType type, std::string lit, size_t row);

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
    std::vector<Token> get_token_vector();

private:

    // Output of lexer vector of tokens
    std::vector<Token> v;
    
    // Vector of strings, each element holds one line
    std::vector<std::string> file_strings;
    
    // Hashmap of keywords (multichar strings) 
    std::map<std::string, TokenType> keyword_map;
    
    // Vector storing corresponding line index 
    size_t row_idx;
    size_t col_idx;
};
