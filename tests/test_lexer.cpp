#include <catch2/catch_test_macros.hpp>
#include <vector>
#include "../src/lexer.hpp"

TEST_CASE("Test single chars"){
    
    Lexer l("tests/lex_files/test_char.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].token_type == TokenType::FUNC);
    REQUIRE(tokens[0].literal == "fn");
}

TEST_CASE("Test add function"){
    
    Lexer add("tests/lex_files/test_add.pragma");

    add.scan();
    auto tokens = add.get_token_vector();
    
    REQUIRE(tokens.size() == 13);
}

TEST_CASE("Matching token vectors"){

    Lexer token_vec("tests/lex_files/test_token_vector.pragma");
    
    token_vec.scan();
    auto tokens = token_vec.get_token_vector();
    
    std::vector<Token> expected{
        
    {TokenType::FUNC, "fn", 0},
    {TokenType::L_PAREN, "(", 0},
    {TokenType::L_PAREN, "(", 0},
    {TokenType::R_PAREN, ")", 0},
    {TokenType::IDENTIFIER, "hello", 2},
    {TokenType::IDENTIFIER, "world", 2},
    {TokenType::PLUS, "+", 2},
    {TokenType::COMMA, ",", 2},
    {TokenType::L_PAREN, "(", 2},
    {TokenType::R_PAREN, ")", 2},
    {TokenType::RETURN, "return", 2},
    {TokenType::INT_LIT, "0", 2},
    {TokenType::PLUS, "+", 4},
    {TokenType::INT_LIT, "2", 4},
    };

    REQUIRE(tokens == expected); 
}
