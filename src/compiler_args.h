#ifndef SGFAULT_ARGS
#define SGFAULT_ARGS
#include "stdio.h"
#include "stdbool.h"
#define NUM_FLAGS 4 //Currently

typedef struct CompilerArgs{
    char *source_file;
    char *output_name;
    char *output_dir;
    FILE *in;
    FILE *out;
    bool verbose;
}CompilerArgs;

CompilerArgs parse_args(int argc, char* argv[]);

void compilation_commands(CompilerArgs* args);
#endif