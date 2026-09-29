#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "../src/parser.hpp"
#include "../src/lexer.hpp"

TEST_CASE("test function parser"){
    
    Lexer l("tests/lex_files/test_add.pragma");

    l.scan();
    auto tokens = l.get_token_vector();
    
    Parser p(tokens);

    std::unique_ptr<func_node> temp = p.funcparse();

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

    std::unique_ptr<func_node> temp = p.funcparse();

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

TEST_CASE("test parallel loop parsing"){

    Lexer l("tests/lex_files/test_parallel.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> temp = p.funcparse();

    auto& body = temp->block_ptr->body_ptrs;
    REQUIRE(body.size() == 3); // a = alloc(...), sum = 0, parallel {...}

    auto* parallel = dynamic_cast<parallel_node*>(body[2].get());
    REQUIRE(parallel != nullptr);
    REQUIRE(parallel->token.literal == "i");
    REQUIRE(parallel->body_ptrs.size() == 1);

    // sum += a[i]
    auto* inner_assign = dynamic_cast<assign_node*>(parallel->body_ptrs[0].get());
    REQUIRE(inner_assign != nullptr);
    REQUIRE(inner_assign->token.literal == "sum");
    REQUIRE(inner_assign->index_ptr == nullptr);
    REQUIRE(inner_assign->op == TokenType::PLUSEQ);

    auto* idx = dynamic_cast<index_node*>(inner_assign->expr_ptr.get());
    REQUIRE(idx != nullptr);
    REQUIRE(idx->token.literal == "a");
}

TEST_CASE("test function call parsing"){

    Lexer l("tests/lex_files/test_call.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::vector<std::unique_ptr<func_node>> funcs = p.programparse();
    REQUIRE(funcs.size() == 3);

    auto& body = funcs[2]->block_ptr->body_ptrs;
    REQUIRE(body.size() == 1);

    // y = add(x, x*2): two args, the second is a full expression
    auto* assign = dynamic_cast<assign_node*>(body[0].get());
    REQUIRE(assign != nullptr);

    auto* call = dynamic_cast<call_node*>(assign->expr_ptr.get());
    REQUIRE(call != nullptr);
    REQUIRE(call->token.literal == "add");
    REQUIRE(call->arg_ptrs.size() == 2);
    REQUIRE(dynamic_cast<var_node*>(call->arg_ptrs[0].get()) != nullptr);

    auto* arg_bin = dynamic_cast<bin_expr_node*>(call->arg_ptrs[1].get());
    REQUIRE(arg_bin != nullptr);
    REQUIRE(arg_bin->operand == TokenType::ASTERISK);

    // return zero() + y: a no-arg call nested inside a binary expression
    auto* ret_bin = dynamic_cast<bin_expr_node*>(funcs[2]->block_ptr->return_ptr->expr_ptr.get());
    REQUIRE(ret_bin != nullptr);

    auto* zero_call = dynamic_cast<call_node*>(ret_bin->var1_ptr.get());
    REQUIRE(zero_call != nullptr);
    REQUIRE(zero_call->token.literal == "zero");
    REQUIRE(zero_call->arg_ptrs.empty());
}
