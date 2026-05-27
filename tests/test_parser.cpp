#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "../src/parser.hpp"
#include "../src/lexer.hpp"

TEST_CASE("test function parser"){
    
    Lexer l("tests/lex_files/test_add.pragma");

    l.scan();
    auto tokens = l.get_token_vector();
    
    Parser p(tokens);

    std::unique_ptr<func_node> temp = std::move(p.funcparse());

    // mock the Tokens at the nodes
    Token func = {TokenType::IDENTIFIER, "add", 1};
    Token param_1= {TokenType::IDENTIFIER, "a", 1};
    Token param_2 = {TokenType::IDENTIFIER, "b", 1};

    REQUIRE(temp->token == func); 
    
    std::vector<std::unique_ptr<param_node>> params = std::move(temp->param_ptrs);

    REQUIRE(params[0]->token == param_1);
    REQUIRE(params[1]->token == param_2);
}
