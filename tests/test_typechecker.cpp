#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "typecheck.hpp"


TEST_CASE("test function with wrong return type is rejected"){

    Lexer l("tests/lex_files/typechecker/test_type_checker_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "function 'add' returns a value of the wrong type");
    CHECK(diags[0].row_idx == 1);
    CHECK(diags[0].severity == Severity::ERROR);
}


TEST_CASE("test valid program produces no diagnostics"){

    Lexer l("tests/lex_files/typechecker/test_type_checker_true.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    CHECK_FALSE(t.get_diagnostics().has_errors());
    CHECK(t.get_diagnostics().get_diagnostics().empty());

    auto& env = t.get_env();
    CHECK(env.at("a") == TokenType::FLOAT_TYPE);
    CHECK(env.at("b") == TokenType::FLOAT_TYPE);
}


TEST_CASE("test array alloc and index typecheck"){

    Lexer l("tests/lex_files/typechecker/test_array_true.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    auto& array_env = t.get_array_env();
    CHECK(array_env.at("a") == TokenType::INT_TYPE);

    auto& env = t.get_env();
    CHECK(env.find("a") == env.end());
}


TEST_CASE("test parallel loop typecheck and reduction clause inference"){

    Lexer l("tests/lex_files/typechecker/test_parallel_true.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    auto& env = t.get_env();
    CHECK(env.at("i") == TokenType::INT_TYPE);
    CHECK(env.at("sum") == TokenType::INT_TYPE);

    auto* parallel = dynamic_cast<parallel_node*>(starting_ptr->block_ptr->body_ptrs[2].get());
    REQUIRE(parallel != nullptr);

    auto& clauses = t.get_parallel_clauses();
    auto found = clauses.find(parallel);

    REQUIRE(found != clauses.end());
    REQUIRE(found->second.size() == 1);
    CHECK(found->second[0] == "reduction(+:sum)");
}


TEST_CASE("test parallel loop typecheck rejects unreduced scalar write"){

    Lexer l("tests/lex_files/typechecker/test_parallel_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "'sum' is written without a reduction operator inside a parallel loop. "
                               "Use += or *= to combine into it safely");
    CHECK(diags[0].row_idx == 5);
}


TEST_CASE("test parallel loop array write with non-loop-variable index"){

    Lexer l("tests/lex_files/typechecker/test_parallel_nonunique_index_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "array write index must be exactly the loop variable 'i' for this parallel loop "
                               "to be provably safe");
    CHECK(diags[0].row_idx == 4);
}


TEST_CASE("test array write to undeclared array"){

    Lexer l("tests/lex_files/typechecker/test_array_not_declared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "'a' is not a declared array");
    CHECK(diags[0].row_idx == 2);
}


TEST_CASE("test array write with non-int index"){

    Lexer l("tests/lex_files/typechecker/test_array_index_not_int_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "array index for 'a' must be of type int");
    CHECK(diags[0].row_idx == 3);
}


TEST_CASE("test compound assignment type mismatch"){

    Lexer l("tests/lex_files/typechecker/test_compound_mismatch_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "value assigned to 'x' does not match its declared type");
    CHECK(diags[0].row_idx == 3);
}


TEST_CASE("test mismatched operand types in return expression"){

    Lexer l("tests/lex_files/typechecker/test_operand_mismatch_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "mismatched operand types around '+'");
    CHECK(diags[1].message == "could not determine the type of the return expression in function 'badoperand'");
}


TEST_CASE("test for loop with undeclared range bound recovers and keeps checking"){

    Lexer l("tests/lex_files/typechecker/test_for_range_undeclared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "'start' is not declared");
    CHECK(diags[1].message == "could not determine the type of the loop range for 'i'");

    // Checker still assumes i:int and finishes checking the body instead of
    // aborting - the loop variable ends up bound despite the earlier error.
    auto& env = t.get_env();
    CHECK(env.at("i") == TokenType::INT_TYPE);
}


TEST_CASE("test checker collects every error instead of stopping at the first"){

    Lexer l("tests/lex_files/typechecker/test_multi_errors_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 3);
    CHECK(diags[0].message == "'y' is not declared");
    CHECK(diags[1].message == "could not determine the type of the expression assigned to 'x'");
    CHECK(diags[2].message == "'z' must be declared before it can be used with a compound assignment");
}
