#include <catch2/catch_test_macros.hpp>
#include <memory>
#include "../src/typecheck.hpp"


TEST_CASE("test function parser"){
    
    Lexer l("tests/lex_files/typechecker/test_type_checker_false.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);

    std::unique_ptr<func_node> starting_ptr = p.funcparse();

    TypeChecker t(std::move(starting_ptr));

    // Not calling t.checker() yet: it reports errors via exit(1), which would
    // kill this whole test binary on this fixture's expected type mismatch.
    // Blocked on the errors-class refactor (see project_typecheck_errors memory).
}
