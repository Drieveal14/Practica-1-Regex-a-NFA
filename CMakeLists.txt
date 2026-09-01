cmake_minimum_required(VERSION 3.16)

project(regex_to_nfa C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(regex_to_nfa
    src/main.c
    src/regex.c
    src/nfa.c
)

target_include_directories(regex_to_nfa PRIVATE
    src
)

target_compile_options(regex_to_nfa PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
