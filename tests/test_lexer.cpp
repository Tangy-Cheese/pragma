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
        
    {TokenType::FUNC, "fn", 1},
    {TokenType::L_PAREN, "(", 1},
    {TokenType::L_PAREN, "(", 1},
    {TokenType::R_PAREN, ")", 1},
    {TokenType::IDENTIFIER, "hello", 3},
    {TokenType::IDENTIFIER, "world", 3},
    {TokenType::PLUS, "+", 3},
    {TokenType::COMMA, ",", 3},
    {TokenType::L_PAREN, "(", 3},
    {TokenType::R_PAREN, ")", 3},
    {TokenType::RETURN, "return", 3},
    {TokenType::INT_LIT, "0", 3},
    {TokenType::PLUS, "+", 5},
    {TokenType::INT_LIT, "2", 5},
    };

    REQUIRE(tokens == expected); 
}
