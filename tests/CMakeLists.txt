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
