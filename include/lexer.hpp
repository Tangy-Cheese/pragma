#pragma once

#include "tokenizer.hpp"
#include <cstddef>
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
     *
     * @param input_file_stream Input file stream to read lines from.
     */
    void conv_string(std::ifstream& input_file_stream);

    /**
     * @brief Simply add the token to the token_vector
     *
     * @param type Token type
     * @param lit Literal string
     * @param row Row of the string
     */
    void push_token(TokenType type, std::string lit, size_t row);

    /**
     * @brief Scans file_strings and separates each line into tokens.
     */
    void scan();

    /**
     * @brief Reading of the token vector.
     *
     * @return Token vector produced by scan().
     */
    std::vector<Token> get_token_vector();

private:

    // Output of lexer vector of tokens
    std::vector<Token> v;
    
    // Vector of strings, each element holds one line
    std::vector<std::string> file_strings;
    
    // Hashmap of keywords (multichar strings) 
    std::map<std::string, TokenType> keyword_map;
};
