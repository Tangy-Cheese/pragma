cmake_minimum_required(VERSION 3.20)
project(pragma
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Applies to every target defined below, including the ones in tests/.
add_compile_options(-Wall -Wextra -Wpedantic)

find_package(OpenMP REQUIRED)
find_package(Catch2 3 REQUIRED)

enable_testing()

add_library(DiagnosticLib STATIC src/diagnostic.cpp)
target_include_directories(DiagnosticLib PUBLIC src)

add_library(LexerLib STATIC src/lexer.cpp)
target_include_directories(LexerLib PUBLIC src)

add_library(ParserLib STATIC src/parser.cpp)
target_include_directories(ParserLib PUBLIC src)
target_link_libraries(ParserLib PRIVATE LexerLib)

add_library(TypecheckLib STATIC src/typecheck.cpp)
target_include_directories(TypecheckLib PUBLIC src)
target_link_libraries(TypecheckLib PRIVATE ParserLib)
target_link_libraries(TypecheckLib PUBLIC DiagnosticLib)

add_library(GeneratorLib STATIC src/generator.cpp)
target_include_directories(GeneratorLib PUBLIC src)
target_link_libraries(GeneratorLib PRIVATE ParserLib)

add_executable(pragc src/main.cpp)
target_link_libraries(pragc PRIVATE LexerLib ParserLib TypecheckLib GeneratorLib DiagnosticLib OpenMP::OpenMP_CXX)

# `cmake --install build` copies pragc to <prefix>/bin (default prefix /usr/local).
install(TARGETS pragc RUNTIME DESTINATION bin)

add_subdirectory(tests)
