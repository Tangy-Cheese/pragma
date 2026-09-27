#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
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


TEST_CASE("pragc compiles a parallel loop end to end, including gcc"){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/parallel.pragma --compile";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream output_file("output.c");
    std::stringstream buffer;
    buffer << output_file.rdbuf();
    std::string output = buffer.str();

    CHECK(output.find("#pragma omp parallel for reduction(+:sum)") != std::string::npos);

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


TEST_CASE("pragc with no --compile does not build output.o"){

    std::remove("output.c");
    std::remove("output.o");

    std::string cmd = std::string(PRAGC_PATH) + " tests/lex_files/generator/sumto.pragma";
    int result = std::system(cmd.c_str());

    REQUIRE(result == 0);

    std::ifstream object_file("output.o");
    CHECK_FALSE(object_file.good());
}
