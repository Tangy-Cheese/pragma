#include "lexer.hpp"
#include "tokenizer.hpp"

#include <cctype>
#include <cstddef>
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


void Lexer::push_token(TokenType type, std::string lit, size_t row){
    v.push_back({type, lit, row});
}


void Lexer::scan(){
    
    for (int row=0; row<Lexer::file_strings.size(); row++) {
           
        // Number of chars in each line
        size_t N = Lexer::file_strings[row].length(); 
        row_idx = row;
        
        std::string row_str = Lexer::file_strings[row];
        std::string buffer;

        int i = 0;
        while (i<N){
            
            char _char = row_str[i];
            
            if (isalpha(_char)){
                while (isalpha(_char) && i<=N){

                    buffer.push_back(_char);

                    i++;
                    _char = row_str[i];
                }
                
                for (auto& literal: keyword_map){

                    if (buffer == literal.first){
                        
                        push_token(literal.second, literal.first, row);
                        buffer.clear();
                        goto skip;
                    }
                }

                // identifier case
                push_token(TokenType::IDENTIFIER, buffer, row);
                buffer.clear();

                skip: ;
            }
           
            else if (isspace(_char)){
                i++; 
                _char = row_str[i];
            }

            else if (std::isdigit(_char)){
             
                while (isdigit(_char) && i<=N){
                    buffer.push_back(_char);

                    i++;
                    _char = row_str[i];
                }

                push_token(TokenType::INT_LIT, buffer, row);
                buffer.clear();
            }
                
    
            else{ 
                switch (_char){            
                    case '(':
                        push_token(TokenType::L_PAREN, "(", row);
                        break;
                    case ')':
                        push_token(TokenType::R_PAREN, ")", row);
                        break;
                    case '{':
                        push_token(TokenType::L_CURLY, "{", row);
                        break;
                    case '}':
                        push_token(TokenType::R_CURLY, "}", row);
                        break;
                    case '+':
                        push_token(TokenType::PLUS, "+", row);
                        break;
                    case ',':
                        push_token(TokenType::COMMA, ",", row);
                        break;
                }
                i++;
            }
        }
    }
}
std::vector<Token> Lexer::get_token_vector(){
    return v;
}
