#include <catch2/catch_test_macros.hpp>
#include "../src/lexer.hpp"

TEST_CASE("Test single chars"){
    
    Lexer l("tests/test_char.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].token_type == TokenType::FUNC);
    REQUIRE(tokens[0].literal == "fn");
}

TEST_CASE("Test add function"){
    
    Lexer add("tests/test_add.pragma");

    add.scan();
    auto tokens = add.get_token_vector();
    
    REQUIRE(tokens.size() == 13);
}
