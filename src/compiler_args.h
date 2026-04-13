#ifndef SGFAULT_ARGS_H
#define SGFAULT_ARGS_H
#include <stdio.h>
#include <stdbool.h>

typedef struct CompilerArgs{
    char *output_path;
    FILE *source_file;
    FILE *out;
    bool verbose;
}CompilerArgs;

CompilerArgs parse_args(int argc, char *argv[]);
void build_binary(CompilerArgs *args);
void free_compiler_args(CompilerArgs *args);
#endif