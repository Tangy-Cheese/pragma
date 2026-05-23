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

add_executable(pragma src/main.cpp)
target_link_libraries(pragma PRIVATE LexerLib OpenMP::OpenMP_CXX)

add_subdirectory(tests)
