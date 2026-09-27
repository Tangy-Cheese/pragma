#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include "generator.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

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
    std::unique_ptr<func_node> ast = p.funcparse();

    TypeChecker t(ast.get());
    t.checker();

    {
        CodeGenerator g;
        g.starting_node = ast.get();
        g.env = t.get_env();
        g.parallel_clauses = t.get_parallel_clauses();
        g.generate();
    }

    std::cout << "Generated output.c" << std::endl;

    if (compile_c){
        // -c only: the parser produces one function per file, not a full
        // program with main(), so there's nothing to link into an executable yet.
        int result = std::system("gcc -std=c99 -fopenmp -c -o output.o output.c");

        if (result != 0){
            std::cerr << "Error: gcc failed to compile output.c." << std::endl;
            return 1;
        }

        std::cout << "Compiled to output.o" << std::endl;
    }

    if (run_bin){
        // Synthesizes a main() that calls the generated function with fixed
        // default arguments and prints its return value. Only scalar params
        // and return types are supported - arrays can't appear in a function
        // signature yet, so there's nothing to synthesize for them.
        std::map<TokenType, std::string> type_string{
            {TokenType::INT_TYPE, "int"},
            {TokenType::DOUBLE_TYPE, "double"},
            {TokenType::FLOAT_TYPE, "float"},
        };

        std::map<TokenType, std::string> default_arg{
            {TokenType::INT_TYPE, "10"},
            {TokenType::DOUBLE_TYPE, "10.0"},
            {TokenType::FLOAT_TYPE, "10.0"},
        };

        std::map<TokenType, std::string> format_spec{
            {TokenType::INT_TYPE, "%d"},
            {TokenType::DOUBLE_TYPE, "%f"},
            {TokenType::FLOAT_TYPE, "%f"},
        };

        if (type_string.find(ast->return_type) == type_string.end()){
            std::cerr << "Error: --run only supports int/float/double return types." << std::endl;
            return 1;
        }

        std::string func_name = ast->token.literal;

        std::ofstream driver("driver.c");
        driver << "#include <stdio.h>\n\n";
        driver << type_string[ast->return_type] << " " << func_name << "(";

        for (size_t i=0; i<ast->param_ptrs.size(); i++){

            if (i>0) driver << ", ";

            auto& param = ast->param_ptrs[i];

            if (type_string.find(param->param_type) == type_string.end()){
                std::cerr << "Error: --run only supports int/float/double parameter types." << std::endl;
                return 1;
            }

            driver << type_string[param->param_type] << " " << param->token.literal;
        }

        driver << ");\n\n";
        driver << "int main(){\n";
        driver << "    " << type_string[ast->return_type] << " result = " << func_name << "(";

        for (size_t i=0; i<ast->param_ptrs.size(); i++){
            if (i>0) driver << ", ";
            driver << default_arg[ast->param_ptrs[i]->param_type];
        }

        driver << ");\n";
        driver << "    printf(\"" << format_spec[ast->return_type] << "\\n\", result);\n";
        driver << "    return 0;\n";
        driver << "}\n";
        driver.close();

        int link_result = std::system("gcc -std=c99 -fopenmp output.c driver.c -o output_bin");

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
