#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "../src/typecheck.hpp"

/**
 * @brief Runs the lexer, parser and type checker over a fixture and returns
 *  every diagnostic collected, so reject-case tests can assert on them.
 *
 * @param path Path to a .pragma file, relative to the repo root.
 * @return Copy of the diagnostics recorded by the type checker.
 */
static std::vector<Diagnostic> diagnose(const std::string& path){

    Lexer l(path);
    l.scan();

    Parser p(l.get_token_vector());
    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    return t.get_diagnostics().get_diagnostics();
}

/**
 * @brief Whether any diagnostic message contains the given substring.
 *
 * @param diags Diagnostics to search.
 * @param needle Substring to look for in each message.
 * @return True if at least one message contains needle.
 */
static bool has_message(const std::vector<Diagnostic>& diags, const std::string& needle){

    for (auto& d: diags){
        if (d.message.find(needle) != std::string::npos) return true;
    }

    return false;
}


TEST_CASE("test function parser"){

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


TEST_CASE("test function typecheck true"){

    Lexer l("tests/lex_files/typechecker/test_type_checker_true.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

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


TEST_CASE("test parallel loop typecheck"){

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

    auto& array_env = t.get_array_env();
    CHECK(array_env.at("a") == TokenType::INT_TYPE);
}


TEST_CASE("test parallel loop reduction clause inference"){

    Lexer l("tests/lex_files/typechecker/test_parallel_true.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

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


TEST_CASE("test return expression with undeclared identifier"){

    Lexer l("tests/lex_files/typechecker/test_return_undeclared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "'x' is not declared");
    CHECK(diags[0].row_idx == 2);
    CHECK(diags[1].message == "could not determine the type of the return expression in function 'badreturn'");
    CHECK(diags[1].row_idx == 1);
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


TEST_CASE("test array element type mismatch on write"){

    Lexer l("tests/lex_files/typechecker/test_array_elem_mismatch_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "value assigned to 'a[...]' does not match its element type");
    CHECK(diags[0].row_idx == 3);
}


TEST_CASE("test array alloc with non-int size"){

    Lexer l("tests/lex_files/typechecker/test_array_size_not_int_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "array size for 'a' must be of type int");
    CHECK(diags[0].row_idx == 2);
}


TEST_CASE("test assignment from undeclared identifier"){

    Lexer l("tests/lex_files/typechecker/test_assign_undeclared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "'y' is not declared");
    CHECK(diags[1].message == "could not determine the type of the expression assigned to 'x'");
}


TEST_CASE("test compound assignment to undeclared identifier"){

    Lexer l("tests/lex_files/typechecker/test_compound_not_declared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 1);
    CHECK(diags[0].message == "'x' must be declared before it can be used with a compound assignment");
    CHECK(diags[0].row_idx == 2);
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


TEST_CASE("test parallel loop with undeclared range bound recovers and keeps checking"){

    Lexer l("tests/lex_files/typechecker/test_parallel_range_undeclared_false.pragma");

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
    CHECK(diags[1].message == "could not determine the type of the parallel loop range for 'i'");

    auto& env = t.get_env();
    CHECK(env.at("i") == TokenType::INT_TYPE);
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


TEST_CASE("test return of undeclared array element"){

    Lexer l("tests/lex_files/typechecker/test_index_expr_undeclared_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(starting_ptr.get());
    t.checker();

    REQUIRE(t.get_diagnostics().has_errors());

    auto& diags = t.get_diagnostics().get_diagnostics();
    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "'b' is not a declared array");
    CHECK(diags[1].message == "could not determine the type of the return expression in function 'badidxexpr'");
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
}


// Parallel safety rejects. Tagged [.known-bug] (hidden from the default run)
// until the OMP analysis pass rejects them.

TEST_CASE("parallel loop rejects read of written array at offset index", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_dep_read_offset_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "array 'a'"));
    CHECK(has_message(diags, "loop variable 'i'"));
}


TEST_CASE("parallel loop rejects read of written array at constant index", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_dep_read_other_idx_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "array 'a'"));
    CHECK(has_message(diags, "loop variable 'i'"));
}


TEST_CASE("parallel loop rejects shared scalar write inside nested loop", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_nested_scalar_race_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'s'"));
    CHECK(has_message(diags, "reduction"));
}


TEST_CASE("parallel loop rejects array write inside nested loop with non-loop-variable index", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_nested_array_race_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "array 'a'"));
    CHECK(has_message(diags, "loop variable 'i'"));
}


TEST_CASE("parallel loop rejects reading a reduction variable", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_reduction_read_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'s'"));
    CHECK(has_message(diags, "read"));
}


TEST_CASE("parallel loop rejects a reduction variable on its own right-hand side", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_reduction_self_read_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'s'"));
    CHECK(has_message(diags, "read"));
}


TEST_CASE("parallel loop rejects mixed reduction operators on one variable", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_reduction_mixed_ops_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'s'"));
    CHECK(has_message(diags, "different"));
}


TEST_CASE("parallel loop rejects plain write to shared scalar", "[parallel]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_plain_write_shared_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'s'"));
    CHECK(has_message(diags, "without a reduction operator"));
}


TEST_CASE("parallel loop rejects assignment to the loop variable", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_write_loop_var_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "cannot assign to"));
    CHECK(has_message(diags, "loop variable 'i'"));

    // The old message wrongly suggested a reduction operator for this case.
    CHECK_FALSE(has_message(diags, "+="));
    CHECK_FALSE(has_message(diags, "*="));
}


TEST_CASE("parallel loop rejects a nested parallel loop", "[parallel][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_nested_parallel_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "nested parallel"));
}


// Scoping and typing rejects. Tagged [.known-bug] until lexical scopes land.

TEST_CASE("variable declared in a for body is not visible after the loop", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_scope_leak_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'t' is not declared"));
}


TEST_CASE("loop variable is not visible after the loop", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_loop_var_leak_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'i' is not declared"));
}


TEST_CASE("re-assigning a variable with a different type is rejected", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_retype_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'x'"));
    CHECK(has_message(diags, "type"));
}


TEST_CASE("for loop bounds must be int", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_float_range_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "must be of type int"));
}


TEST_CASE("parallel loop bounds must be int", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_float_range_parallel_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "must be of type int"));
}


TEST_CASE("assigning a scalar to an array name is rejected", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_array_to_scalar_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'a'"));
    CHECK(has_message(diags, "array"));
}


TEST_CASE("re-declaring a scalar as an array is rejected", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_scalar_to_array_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'a'"));
    CHECK(has_message(diags, "already declared"));
}


TEST_CASE("loop variable may not shadow an existing name", "[scope][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_loop_var_shadows_array_false.pragma");

    REQUIRE_FALSE(diags.empty());
    CHECK(has_message(diags, "'a'"));
    CHECK(has_message(diags, "shadow"));
}


TEST_CASE("undeclared operands are each reported once, with no cascading error", "[diagnostics][.known-bug]"){

    auto diags = diagnose("tests/lex_files/typechecker/test_double_error_false.pragma");

    REQUIRE(diags.size() == 2);
    CHECK(diags[0].message == "'x' is not declared");
    CHECK(diags[1].message == "'y' is not declared");
    CHECK_FALSE(has_message(diags, "could not determine"));
}
