#include "lexer.hpp"
#include "tokenizer.hpp"

#include <cctype>
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
    std::string buffer;

    for (int i=0; i<N; i++){
        
        char _character = Lexer::file_string[i];
        
        if (std::isalpha(_character)){
            while (isalpha(_character)){
                buffer.push_back(_character);

                i++;

                if (buffer == "return"){
                    v.push_back(Group(TokenType::RETURN, "return", i, j));
                    buffer.clear();
                }   
            }
        }

        else if (std::isspace(_character)){
         
            while (isalpha(_character)){
                buffer.push_back(_character);

                i++;

                if (buffer == "return"){
                    v.push_back(Group(TokenType::RETURN, "return", i, j));
                    buffer.clear();
                }   
            }
        }

        
        else if (std::isspace(_character)){
         
            while (isalpha(_character)){
                buffer.push_back(_character);

                i++;

                if (buffer == "return"){
                    v.push_back(Group(TokenType::RETURN, "return", i, j));
                    buffer.clear();
                }   
            }
        }

        else if (std::isspace(_character)){
         
            while (isalpha(_character)){
                buffer.push_back(_character);

                i++;

                if (buffer == "return"){
                    v.push_back(Group(TokenType::RETURN, "return", i, j));
                    buffer.clear();
                }   
            }
        }

        else{         
            while (isalpha(_character)){
                buffer.push_back(_character);

                i++;

                if (buffer == "return"){
                    v.push_back(Group(TokenType::RETURN, "return", i, j));
                    buffer.clear();
                }   
            }
        }
    }
}

std::vector<Group> Lexer::get_token_vector(){
    return v;
}
