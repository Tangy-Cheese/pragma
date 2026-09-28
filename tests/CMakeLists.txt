add_executable(TestLexer test_lexer.cpp)
target_link_libraries(TestLexer PRIVATE LexerLib Catch2::Catch2WithMain)

add_executable(TestParser test_parser.cpp)
target_link_libraries(TestParser PRIVATE ParserLib Catch2::Catch2WithMain)

add_executable(TestTypechecker test_typechecker.cpp)
target_link_libraries(TestTypechecker PRIVATE TypecheckLib Catch2::Catch2WithMain)

add_executable(TestGenerator test_generator.cpp)
target_link_libraries(TestGenerator PRIVATE TypecheckLib GeneratorLib Catch2::Catch2WithMain)

add_executable(TestPragc test_pragc.cpp)
target_link_libraries(TestPragc PRIVATE Catch2::Catch2WithMain)
target_compile_definitions(TestPragc PRIVATE PRAGC_PATH="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/pragc")
add_dependencies(TestPragc pragc)

# Tests open fixtures like tests/lex_files/... relative to the repo root, so
# every test runs from there instead of ctest's default (the build directory).
foreach(name Lexer Parser Typechecker Generator Pragc)
    add_test(NAME ${name} COMMAND Test${name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()

# Generator and Pragc both write and read back ./output.c, so running them
# concurrently under ctest -j would race on that one file.
set_tests_properties(Generator Pragc PROPERTIES RUN_SERIAL TRUE)

# End-to-end: generated C must give the sequential answer at 1 and 8 threads.
# Works in a temp directory, so it doesn't share output.c with the tests above.
add_test(NAME Runtime
    COMMAND bash tests/runtime/run.sh $<TARGET_FILE:pragc>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
