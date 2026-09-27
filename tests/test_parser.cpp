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
    
    REQUIRE(params[0]->param_type == TokenType::INT_TYPE);
    REQUIRE(params[1]->param_type == TokenType::INT_TYPE);


    //func->params/block->return->expr->var 

    REQUIRE(temp->block_ptr->body_ptrs.size() == 0);
    REQUIRE(temp->block_ptr->return_ptr != nullptr);

    auto* bin = dynamic_cast<bin_expr_node*>(temp->block_ptr->return_ptr->expr_ptr.get());
    REQUIRE(bin != nullptr);
    REQUIRE(bin->operand == TokenType::PLUS);

    Token var_a = {TokenType::IDENTIFIER, "a", 2};
    Token var_b = {TokenType::IDENTIFIER, "b", 2};
    REQUIRE(bin->var1_ptr->token == var_a);
    REQUIRE(bin->var2_ptr->token == var_b);
}

TEST_CASE("test array alloc and index parsing"){

    Lexer l("tests/lex_files/test_array.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> temp = std::move(p.funcparse());

    auto& body = temp->block_ptr->body_ptrs;
    REQUIRE(body.size() == 2);

    // a = alloc(int, n)
    auto* alloc_assign = dynamic_cast<assign_node*>(body[0].get());
    REQUIRE(alloc_assign != nullptr);
    REQUIRE(alloc_assign->index_ptr == nullptr);

    auto* alloc = dynamic_cast<alloc_node*>(alloc_assign->expr_ptr.get());
    REQUIRE(alloc != nullptr);
    REQUIRE(alloc->elem_type == TokenType::INT_TYPE);

    // a[0] = n
    auto* index_assign = dynamic_cast<assign_node*>(body[1].get());
    REQUIRE(index_assign != nullptr);
    REQUIRE(index_assign->token.literal == "a");
    REQUIRE(index_assign->index_ptr != nullptr);

    // return a[0]
    auto* ret_index = dynamic_cast<index_node*>(temp->block_ptr->return_ptr->expr_ptr.get());
    REQUIRE(ret_index != nullptr);
    REQUIRE(ret_index->token.literal == "a");
}
