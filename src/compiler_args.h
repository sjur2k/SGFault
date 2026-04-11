#ifndef SGFAULT_ARGS_H
#define SGFAULT_ARGS_H
#include <stdio.h>
#include <stdbool.h>
#define NUM_OPTIONS 3

typedef struct CompilerArgs{
    char *output_path;
    FILE *source_file;
    FILE *out;
    bool verbose;
}CompilerArgs;

CompilerArgs parse_args(int argc, char *argv[]);
void compilation_commands(CompilerArgs *args);
void compiler_args_free(CompilerArgs *args);
#endif