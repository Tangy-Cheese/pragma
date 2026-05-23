add_executable(TestLexer test_lexer.cpp)
target_link_libraries(TestLexer PRIVATE LexerLib Catch2::Catch2WithMain)
