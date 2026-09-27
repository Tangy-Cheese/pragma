#include "lexer.hpp"
#include "tokenizer.hpp"

#include <cctype>
#include <cstddef>
#include <iostream>
#include <fstream>

Lexer::Lexer(std::string file_name){

    std::ifstream input_stream(file_name);

    // check stream status
    if (!input_stream) std::cerr << "Error: could not open input file '" << file_name << "'." << std::endl;

    conv_string(input_stream);

    // Build the hashmap
    keyword_map.insert({"return",TokenType::RETURN});
    keyword_map.insert({"fn",TokenType::FUNC});
    keyword_map.insert({"out",TokenType::PRINT});
    keyword_map.insert({"for",TokenType::FOR});
    keyword_map.insert({"in",TokenType::IN});
    keyword_map.insert({"alloc",TokenType::ALLOC});
    keyword_map.insert({"parallel",TokenType::PARALLEL});

    //Types
    keyword_map.insert({"int",TokenType::INT_TYPE});
    keyword_map.insert({"complex",TokenType::COMPLEX_TYPE});
    keyword_map.insert({"bool",TokenType::BOOL_TYPE});
    keyword_map.insert({"double",TokenType::DOUBLE_TYPE});
    keyword_map.insert({"float",TokenType::FLOAT_TYPE});
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
    
    for (size_t row=0; row<Lexer::file_strings.size(); row++) {
           
        // Number of chars in each line
        size_t N = Lexer::file_strings[row].length(); 
        
        std::string row_str = Lexer::file_strings[row];
        std::string buffer;

        size_t i = 0;
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
                        
                        push_token(literal.second, literal.first, row+1);
                        buffer.clear();
                        goto skip;
                    }
                }

                // identifier case
                push_token(TokenType::IDENTIFIER, buffer, row+1);
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

                bool is_float = false;

                // Only consume the '.' as a decimal point if a digit follows it;
                // otherwise it's the start of a range's '..' (e.g. 0..n).
                if (_char == '.' && i+1<N && isdigit(row_str[i+1])){
                    is_float = true;
                    buffer.push_back(_char);
                    i++;
                    _char = row_str[i];

                    while (isdigit(_char) && i<=N){
                        buffer.push_back(_char);

                        i++;
                        _char = row_str[i];
                    }
                }

                push_token(is_float ? TokenType::FLOAT_LIT : TokenType::INT_LIT, buffer, row+1);
                buffer.clear();
            }
                
    
            else{ 
                switch (_char){            
                    case '(':
                        push_token(TokenType::L_PAREN, "(", row+1);
                        break;
                    case ')':
                        push_token(TokenType::R_PAREN, ")", row+1);
                        break;
                    case '{':
                        push_token(TokenType::L_CURLY, "{", row+1);
                        break;
                    case '}':
                        push_token(TokenType::R_CURLY, "}", row+1);
                        break;
                    case '[':
                        push_token(TokenType::L_BRACKET, "[", row+1);
                        break;
                    case ']':
                        push_token(TokenType::R_BRACKET, "]", row+1);
                        break;
                    case '+':
                        if (i+1<N && row_str[i+1]=='='){
                            push_token(TokenType::PLUSEQ, "+=", row+1);
                            i++;
                        }
                        else{
                            push_token(TokenType::PLUS, "+", row+1);
                        }
                        break;
                    case '-':
                        if (i+1<N && row_str[i+1]=='='){
                            push_token(TokenType::MINUSEQ, "-=", row+1);
                            i++;
                        }
                        else{
                            push_token(TokenType::MINUS, "-", row+1);
                        }
                        break;
                    case '*':
                        if (i+1<N && row_str[i+1]=='='){
                            push_token(TokenType::ASTERISKEQ, "*=", row+1);
                            i++;
                        }
                        else{
                            push_token(TokenType::ASTERISK, "*", row+1);
                        }
                        break;
                    case '/':
                        if (i+1<N && row_str[i+1]=='='){
                            push_token(TokenType::SLASHEQ, "/=", row+1);
                            i++;
                        }
                        else{
                            push_token(TokenType::SLASH, "/", row+1);
                        }
                        break;
                    case '%':
                        push_token(TokenType::PERCENT, "%", row+1);
                        break;
                    case ',':
                        push_token(TokenType::COMMA, ",", row+1);
                        break;
                    case ':':
                        push_token(TokenType::COLON, ":", row+1);
                        break;
                    case '=':
                        push_token(TokenType::EQUALS, "=", row+1);
                        break;
                    case '.':
                        if (i+1<N && row_str[i+1]=='.'){
                            push_token(TokenType::RANGE, "..", row+1);
                            i++;
                        }
                        else{
                            std::cerr << "Error: unexpected character '.' (row " << row+1 << ")." << std::endl;
                        }
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
