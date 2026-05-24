#include "lexer.hpp"
#include "tokenizer.hpp"

#include <cctype>
#include <iostream>
#include <fstream>

Lexer::Lexer(std::string file_name){

    std::ifstream input_stream(file_name);

    // check stream status
    if (!input_stream) std::cerr << "Can't open input file!";

    conv_string(input_stream);

    // Build the hashmap
    keyword_map.insert({"return",TokenType::RETURN});
    keyword_map.insert({"fn",TokenType::FUNC});
    keyword_map.insert({"out",TokenType::PRINT});
}

Lexer::~Lexer(){}


void Lexer::conv_string(std::ifstream& input){

    // one line
    std::string line;
    
    // extract all the text from the input file
    while (getline(input, line)) {

        // store each line in the vector along with counter
        Lexer::file_strings.push_back(line);
    }
}

void Lexer::scan(){
    
    for (int row=0; row<Lexer::file_strings.size(); row++) {
           
        // Number of chars in each line
        size_t N = Lexer::file_strings[row].length(); 
        row_idx = row;
        
        std::string row_str = Lexer::file_strings[row];
        std::string buffer;

        for (int i=0; i<N; i++){
            
            char _char = row_str[i];
            
            if (std::isalpha(_char)){
                while (isalpha(_char)){

                    buffer.push_back(_char);

                    i++;
                    _char = row_str[i];
                }
                
                for (auto& literal: keyword_map){

                    if (buffer == literal.first){
                        
                        Group s;
                        s.token_type = literal.second;
                        s.literal = literal.first;
                        s.row_idx = row;

                        v.push_back(s);
                        buffer.clear();
                    }

                    else{ // Non keyword case
                        Group s;
                        s.token_type = TokenType::IDENTIFIER;
                        s.literal = buffer;
                        s.row_idx = row;
                        buffer.clear();
                    }
                }
            }
           
            else if (std::isspace(_char)){
             
                while (isalpha(_char)){
                    buffer.push_back(_char);

                    i++;

                    if (buffer == "return"){
                        v.push_back(Group(TokenType::RETURN, "return", row));
                        buffer.clear();
                    }   
                    i--;
                }
            }

            else if (std::isdigit(_char)){
             
                while (isdigit(_char)){
                    buffer.push_back(_char);

                    i++;

                    if (buffer == "return"){
                        v.push_back(Group(TokenType::RETURN, "return", row));
                        buffer.clear();
                    }   
                }
            }
            
            // identifiers/var names
            else{         
                while (isalpha(_char)){
                    buffer.push_back(_char);

                    i++;

                    if (buffer == "return"){
                        v.push_back(Group(TokenType::RETURN, "return", row));
                        buffer.clear();
                    }   
                }
            }
        }
    }
}


std::vector<Group> Lexer::get_token_vector(){
    return v;
}
