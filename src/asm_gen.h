#ifndef SGFAULT_ASSEMBLY
#define SGFAULT_ASSEMBLY

#include <stdio.h>
#include "parser.h"

typedef struct{
    FILE* out;
    ASTList trees;
}AsmContext;

void generate_asm(AsmContext *context);

AsmContext create_asm_context(FILE *out, ASTList AST_list);

#endif