#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include "generator.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char** argv){

    if (argc < 2){
        std::cerr << "Usage: pragc <input.pragma> [--compile]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    bool compile_c = false;

    for (int i=2; i<argc; i++){
        if (std::string(argv[i]) == "--compile"){
            compile_c = true;
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
        g.generate();
    }

    std::cout << "Generated output.c" << std::endl;

    if (compile_c){
        // -c only: the parser produces one function per file, not a full
        // program with main(), so there's nothing to link into an executable yet.
        int result = std::system("gcc -std=c99 -c -o output.o output.c");

        if (result != 0){
            std::cerr << "C compilation failed" << std::endl;
            return 1;
        }

        std::cout << "Compiled to output.o" << std::endl;
    }

    return 0;
}
