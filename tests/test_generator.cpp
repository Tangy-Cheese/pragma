#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <fstream>
#include <sstream>
#include "../src/generator.hpp"
#include "../src/typecheck.hpp"


TEST_CASE("test codegen for assignment and for loop"){

    Lexer l("tests/lex_files/generator/sumto.pragma");

    l.scan();
    auto tokens = l.get_token_vector();

    // Separate parses for typecheck and codegen: each owns its own AST.
    Parser p(tokens);
    std::unique_ptr<func_node> ast = p.funcparse();

    Parser p_check(tokens);
    std::unique_ptr<func_node> ast_for_check = p_check.funcparse();

    TypeChecker t(std::move(ast_for_check));
    t.checker();

    {
        // CodeGenerator's destructor closes out_stream, flushing to disk,
        // so it must go out of scope before output.c is read back below.
        CodeGenerator g;
        g.starting_node = std::move(ast);
        g.env = t.get_env();
        g.generate();
    }

    std::ifstream output_file("output.c");
    std::stringstream buffer;
    buffer << output_file.rdbuf();
    std::string output = buffer.str();

    CHECK(output.find("int sumto(int n){") != std::string::npos);
    CHECK(output.find("int sum = 0;") != std::string::npos);
    CHECK(output.find("for (int i = 0; i < n; i++){") != std::string::npos);
    CHECK(output.find("sum = (sum + i);") != std::string::npos);
    CHECK(output.find("return sum;") != std::string::npos);
}
