#include <catch2/catch_test_macros.hpp>
#include <algorithm>
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

TEST_CASE("Test underscores in identifiers"){

    Lexer l("tests/lex_files/test_underscore.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].token_type == TokenType::IDENTIFIER);
    REQUIRE(tokens[0].literal == "sum_squares");
    REQUIRE(tokens[1].literal == "_tmp");
    REQUIRE(tokens[2].literal == "a_b_");
    REQUIRE(tokens[3].token_type == TokenType::IDENTIFIER);
    REQUIRE(tokens[3].literal == "fn_x");
}

TEST_CASE("Test add function"){
    
    Lexer add("tests/lex_files/test_add.pragma");

    add.scan();
    auto tokens = add.get_token_vector();
    
    REQUIRE(tokens.size() == 19);
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

TEST_CASE("Test array tokens"){

    Lexer l("tests/lex_files/test_array.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    REQUIRE(tokens.size() == 30);

    Token alloc_tok = {TokenType::ALLOC, "alloc", 2};
    Token l_bracket = {TokenType::L_BRACKET, "[", 3};
    Token r_bracket = {TokenType::R_BRACKET, "]", 3};

    REQUIRE(tokens[12] == alloc_tok);
    REQUIRE(tokens[19] == l_bracket);
    REQUIRE(tokens[21] == r_bracket);
}

TEST_CASE("Test float literal lexing and range disambiguation"){

    Lexer l("tests/lex_files/test_float.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    std::vector<Token> expected{

    {TokenType::IDENTIFIER, "x", 1},
    {TokenType::EQUALS, "=", 1},
    {TokenType::FLOAT_LIT, "3.14", 1},
    {TokenType::IDENTIFIER, "y", 2},
    {TokenType::EQUALS, "=", 2},
    {TokenType::INT_LIT, "0", 2},
    {TokenType::RANGE, "..", 2},
    {TokenType::INT_LIT, "5", 2},
    };

    REQUIRE(tokens == expected);
}

TEST_CASE("Test parallel loop tokens"){

    Lexer l("tests/lex_files/test_parallel.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    auto it = std::find_if(tokens.begin(), tokens.end(), [](const Token& t){
        return t.token_type == TokenType::PARALLEL;
    });

    REQUIRE(it != tokens.end());
    REQUIRE(it->literal == "parallel");

    // parallel i in 0..n {
    Token loop_var = *(it + 1);
    Token in_tok = *(it + 2);

    REQUIRE(loop_var.token_type == TokenType::IDENTIFIER);
    REQUIRE(loop_var.literal == "i");
    REQUIRE(in_tok.token_type == TokenType::IN);
}
