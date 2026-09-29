#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// Deletes the given repo-root files when the test case scope exits, whether
// it passes, fails, or a REQUIRE throws partway through - stack unwinding
// still runs local destructors, so cleanup happens either way.
struct FileCleanup{
    std::vector<std::string> paths;

    ~FileCleanup(){
        for (auto& path: paths) std::remove(path.c_str());
    }
};


/**
 * @brief Runs pragc on a fixture, requires it to succeed, then compiles the
 *  generated C with the strict flags the generated code is expected to meet.
 *  Callers must clean up output.c and output.o.
 *
 * @param fixture Path to a .pragma file, relative to the repo root.
 * @return Contents of the generated output.c.
 */
static std::string generate_and_compile_strict(const std::string& fixture){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " " + fixture;
    REQUIRE(std::system(cmd.c_str()) == 0);

    REQUIRE(std::system("gcc -std=c99 -fopenmp -Wall -Werror -c output.c -o output.o") == 0);

    std::ifstream output_file("output.c");
    std::stringstream buffer;
    buffer << output_file.rdbuf();
    return buffer.str();
}

/**
 * @brief Number of non-overlapping occurrences of needle in text.
 */
static int count_occurrences(const std::string& text, const std::string& needle){

    int count = 0;

    for (size_t pos = text.find(needle); pos != std::string::npos; pos = text.find(needle, pos + needle.size())){
        count++;
    }

    return count;
}


TEST_CASE("pragc compiles a .pragma file to output.c"){

    std::remove("output.c");
    FileCleanup cleanup{{"output.c"}};

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
    FileCleanup cleanup{{"output.c", "output.o"}};

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/sumto.pragma --compile";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream object_file("output.o");
    CHECK(object_file.good());
}


TEST_CASE("benchmark: sequential vs parallel reduction on a large sum"){

    // Report-only: prints timings and the speedup ratio for manual inspection.
    // Does NOT assert parallel is faster - on a shared/loaded machine (not a
    // dedicated CI box) wall-clock speedup is affected by memory pressure and
    // contention from unrelated processes, which is outside this test's
    // control and would make a hard timing assertion genuinely flaky rather
    // than a real regression signal. What IS asserted: both versions must
    // compute the identical result, since that's a real invariant.

    std::remove("output.c");
    std::remove("output_seq.c");
    std::remove("output_par.c");
    std::remove("bench_driver.c");
    std::remove("bench_bin");
    std::remove("bench_out.txt");
    FileCleanup cleanup{{"output.c", "output_seq.c", "output_par.c", "bench_driver.c", "bench_bin", "bench_out.txt"}};

    std::string seq_cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/seq_bench.pragma";
    REQUIRE(std::system(seq_cmd.c_str()) == 0);
    std::rename("output.c", "output_seq.c");

    std::string par_cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/par_bench.pragma";
    REQUIRE(std::system(par_cmd.c_str()) == 0);
    std::rename("output.c", "output_par.c");

    std::ofstream driver("bench_driver.c");
    driver << "#include <stdio.h>\n";
    driver << "#include <omp.h>\n\n";
    driver << "extern int seqsum(int n);\n";
    driver << "extern int parsumbench(int n);\n\n";
    driver << "int main(){\n";
    driver << "    int n = 10000000;\n";
    driver << "    int trials = 3;\n";
    driver << "    double seq_best = 1e18, par_best = 1e18;\n";
    driver << "    int seq_result = 0, par_result = 0;\n\n";
    driver << "    for (int t = 0; t < trials; t++){\n";
    driver << "        double t0 = omp_get_wtime();\n";
    driver << "        seq_result = seqsum(n);\n";
    driver << "        double t1 = omp_get_wtime();\n";
    driver << "        if (t1 - t0 < seq_best) seq_best = t1 - t0;\n";
    driver << "    }\n\n";
    driver << "    for (int t = 0; t < trials; t++){\n";
    driver << "        double t0 = omp_get_wtime();\n";
    driver << "        par_result = parsumbench(n);\n";
    driver << "        double t1 = omp_get_wtime();\n";
    driver << "        if (t1 - t0 < par_best) par_best = t1 - t0;\n";
    driver << "    }\n\n";
    driver << "    printf(\"%d %d %d %f %f\\n\", omp_get_max_threads(), seq_result, par_result, seq_best, par_best);\n";
    driver << "    return 0;\n";
    driver << "}\n";
    driver.close();

    int link_result = std::system("gcc -std=c99 -fopenmp output_seq.c output_par.c bench_driver.c -o bench_bin");
    REQUIRE(link_result == 0);

    REQUIRE(std::system("./bench_bin > bench_out.txt") == 0);

    std::ifstream result_file("bench_out.txt");
    int threads = 0, seq_result = 0, par_result = 0;
    double seq_time = 0, par_time = 0;

    bool parsed = bool(result_file >> threads >> seq_result >> par_result >> seq_time >> par_time);
    REQUIRE(parsed);

    CHECK(seq_result == par_result);

    double speedup = seq_time / par_time;

    std::cout << "[benchmark] threads available: " << threads << "\n"
               << "[benchmark] sequential best of 3: " << seq_time << "s\n"
               << "[benchmark] parallel best of 3:   " << par_time << "s\n"
               << "[benchmark] speedup: " << speedup << "x (informational only, not asserted)\n";
}


// Accept cases: each program must pass every check, generate C that compiles
// under -Wall -Werror, and carry exactly the expected OpenMP clause.

TEST_CASE("a written array indexed by the loop variable in a nested loop is accepted", "[parallel]"){

    FileCleanup cleanup{{"output.c", "output.o"}};

    std::string output = generate_and_compile_strict("tests/lex_files/typechecker/test_nested_array_ok_true.pragma");

    CHECK(count_occurrences(output, "#pragma omp parallel for\n") == 1);
    CHECK(count_occurrences(output, "reduction") == 0);
}


TEST_CASE("arrays that are only read may use any index", "[parallel]"){

    FileCleanup cleanup{{"output.c", "output.o"}};

    std::string output = generate_and_compile_strict("tests/lex_files/typechecker/test_read_only_array_any_index_true.pragma");

    CHECK(count_occurrences(output, "#pragma omp parallel for\n") == 1);
    CHECK(count_occurrences(output, "reduction") == 0);
}


TEST_CASE("a float reduction gets a reduction clause", "[parallel]"){

    FileCleanup cleanup{{"output.c", "output.o"}};

    std::string output = generate_and_compile_strict("tests/lex_files/typechecker/test_float_reduction_true.pragma");

    CHECK(count_occurrences(output, "reduction(+:s)") == 1);
}
