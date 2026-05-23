#include "lexer.hpp"
#include "tokenizer.hpp"

#include <iostream>

Lexer::Lexer(){

    // define your file name
    std::string file_name = "data.pragma";
    std::ifstream input_stream(file_name);

    // check stream status
    if (!input_stream) std::cerr << "Can't open input file!";
    std::vector<std::string> text;

    // one line
    std::string line;

    // extract all the text from the input file
    while (getline(input_stream, line)) {

        // store each line in the vector
        text.push_back(line);
}


void Lexer::conv_string(){

}

void Lexer::get_token_vector(){

}
