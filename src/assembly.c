#include "assembly.h"
#include <stdio.h>
#include "parser.h"

void generate_asm(AsmContext *context){
    //PLACEHOLDER CODE:
    fprintf(context->out,"global _start\n\n");
    fprintf(context->out,"_start:\n");
    fprintf(context->out,"\tmov rdi, 5\n");
    fprintf(context->out,"\tmov rax, 60\n");
    fprintf(context->out,"\tsyscall");
    
    fclose(context->out);
    context->out = NULL;
}

AsmContext create_asm_context(FILE* out, ASTList AST_list){
    return (AsmContext){
        .out = out,
        .trees = AST_list,
    };
}