cmake_minimum_required(VERSION 3.10)

project(SGFault C)
set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Wextra -Wpedantic -Wstrict-prototypes)

add_executable(sgfault
    src/main.c
    src/utils.c
    src/lexer.c
    src/compiler_args.c
    src/parser.c
)

target_compile_definitions(sgfault PRIVATE
    DATA_DIR="${CMAKE_INSTALL_PREFIX}/share"
)

install(TARGETS sgfault DESTINATION bin)
install(DIRECTORY docs/ DESTINATION share/docs)
