cmake_minimum_required(VERSION 3.20)
project(pragma
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(OpenMP REQUIRED)
find_package(Catch2 3 REQUIRED)

add_library(LexerLib STATIC src/lexer.cpp)
target_include_directories(LexerLib PUBLIC src)

add_library(ParserLib STATIC src/parser.cpp)
target_include_directories(ParserLib PUBLIC src)
target_link_libraries(ParserLib PRIVATE LexerLib)

add_executable(pragc src/main.cpp)
target_link_libraries(pragc PRIVATE LexerLib ParserLib OpenMP::OpenMP_CXX)

add_subdirectory(tests)
