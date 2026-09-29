#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include "generator.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <vector>

int main(int argc, char** argv){

    if (argc < 2){
        std::cerr << "Usage: pragc <input.pragma> [--compile] [--run]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    bool compile_c = false;
    bool run_bin = false;

    for (int i=2; i<argc; i++){
        if (std::string(argv[i]) == "--compile"){
            compile_c = true;
        }
        else if (std::string(argv[i]) == "--run"){
            run_bin = true;
        }
    }

    Lexer l(input_file);
    l.scan();
    auto tokens = l.get_token_vector();

    Parser p(tokens);
    std::vector<std::unique_ptr<func_node>> funcs = p.programparse();
    std::unique_ptr<func_node> main_func = p.take_main();

    // Program-level checks that span functions, separate from each TypeChecker's own.
    DiagnosticCollector program_diagnostics;
    std::set<std::string> func_names;

    for (auto& func: funcs){
        if (func->token.literal == "main"){
            program_diagnostics.add(func->token.row_idx,
                "'main' is reserved: code outside any function becomes main");
        }
        else if (!func_names.insert(func->token.literal).second){
            program_diagnostics.add(func->token.row_idx,
                "duplicate function name '" + func->token.literal + "'");
        }
    }

    // Added after the name checks, so the generated main isn't flagged as a user's main.
    bool has_main = main_func != nullptr;
    if (main_func) funcs.push_back(std::move(main_func));

    if (funcs.empty()){
        std::cerr << "Error: nothing to compile." << std::endl;
        return 1;
    }

    // Checkers outlive this loop: codegen needs each function's own env and clauses.
    std::vector<std::unique_ptr<TypeChecker>> checkers;
    bool has_errors = program_diagnostics.has_errors();

    // First pass: every signature is known before any body is checked.
    auto signatures = collect_signatures(funcs);

    for (auto& func: funcs){
        checkers.push_back(std::make_unique<TypeChecker>(func.get(), signatures));
        checkers.back()->checker();
        has_errors = has_errors || checkers.back()->get_diagnostics().has_errors();
    }

    if (has_errors){
        program_diagnostics.print_all();
        for (auto& t: checkers){
            t->get_diagnostics().print_all();
        }
        return 1;
    }

    {
        std::vector<func_unit> units;

        for (size_t i=0; i<funcs.size(); i++){
            units.push_back({funcs[i].get(), checkers[i]->get_env(), checkers[i]->get_parallel_clauses()});
        }

        CodeGenerator g;
        g.generate_program(units);
    }

    std::cout << "Generated output.c" << std::endl;

    if (compile_c){
        // -c: a file without top-level code has no main, so only an object file is always possible.
        int result = std::system("gcc -std=c99 -fopenmp -c -o output.o output.c");

        if (result != 0){
            std::cerr << "Error: gcc failed to compile output.c." << std::endl;
            return 1;
        }

        std::cout << "Compiled to output.o" << std::endl;
    }

    if (run_bin){

        if (!has_main){
            std::cerr << "Error: nothing to run. --run needs code outside any function, "
                         "which becomes main." << std::endl;
            return 1;
        }

        int link_result = std::system("gcc -std=c99 -fopenmp output.c -o output_bin");

        if (link_result != 0){
            std::cerr << "Error: gcc failed to link output_bin." << std::endl;
            return 1;
        }

        std::cout << "Running output_bin:" << std::endl;

        int run_result = std::system("./output_bin");

        if (run_result != 0){
            std::cerr << "Error: output_bin exited with a nonzero status." << std::endl;
            return 1;
        }
    }

    return 0;
}
