#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>


TEST_CASE("pragc compiles a .pragma file to output.c"){

    std::remove("output.c");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/sumto.pragma";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream output_file("output.c");
    std::stringstream buffer;
    buffer << output_file.rdbuf();
    std::string output = buffer.str();

    CHECK(output.find("int sumto(int n){") != std::string::npos);
    CHECK(output.find("for (int i = 0; i < n; i++){") != std::string::npos);
    CHECK(output.find("return sum;") != std::string::npos);
}


TEST_CASE("pragc --compile also builds output.o"){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/sumto.pragma --compile";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream object_file("output.o");
    CHECK(object_file.good());
}


TEST_CASE("pragc compiles arrays end to end, including gcc"){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/array.pragma --compile";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream output_file("output.c");
    std::stringstream buffer;
    buffer << output_file.rdbuf();
    std::string output = buffer.str();

    CHECK(output.find("int* a = malloc(n * sizeof(int));") != std::string::npos);
    CHECK(output.find("a[i] = i;") != std::string::npos);

    std::ifstream object_file("output.o");
    CHECK(object_file.good());
}


TEST_CASE("pragc with no --compile does not build output.o"){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/sumto.pragma";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream object_file("output.o");
    CHECK_FALSE(object_file.good());
}
