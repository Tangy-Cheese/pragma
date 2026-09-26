add_executable(TestLexer test_lexer.cpp)
target_link_libraries(TestLexer PRIVATE LexerLib Catch2::Catch2WithMain)

add_executable(TestParser test_parser.cpp)
target_link_libraries(TestParser PRIVATE ParserLib Catch2::Catch2WithMain)

add_executable(TestTypechecker test_typechecker.cpp ../src/typecheck.cpp)
target_link_libraries(TestTypechecker PRIVATE ParserLib Catch2::Catch2WithMain)
