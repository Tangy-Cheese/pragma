#include "lexer.hpp"

#include <iostream>
#include <fstream>

Lexer::Lexer(){

    // define your file name
    std::string file_name = "file.pragma";
    std::ifstream input_stream(file_name);

    // check stream status
    if (!input_stream) std::cerr << "Can't open input file!";

    conv_string(input_stream);
}

void Lexer::conv_string(std::ifstream& input){

    std::vector<std::string> text;

    // one line
    std::string line;

    // extract all the text from the input file
    while (getline(input, line)) {

        // store each line in the vector
        Lexer::file_string.append(line);
    }
}

void Lexer::scan(){

    size_t N = Lexer::file_string.length();

    for (int i=0; i<N; i++){

    }
}

std::vector<Group> Lexer::get_token_vector(){
    return v;
}
