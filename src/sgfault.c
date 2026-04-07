#include "compiler_args.h"
#include "lexer.h"

void generate_asm(TokenList* t_list, CompilerArgs* args){
    //PLACEHOLDER CODE:
    int num_tokens = t_list->size;
    printf("Number of tokens: %d\n",num_tokens);
    fprintf(args->out,"global _start\n\n");
    fprintf(args->out,"_start:\n");
    fprintf(args->out,"\tmov rdi, 0\n");
    fprintf(args->out,"\tmov rax, 60\n");
    fprintf(args->out,"\tsyscall");
    fclose(args->out)
    
    args->out = NULL
}

int main(int argc, char *argv[]){
    CompilerArgs args = parse_args(argc,argv);
    

    TokenList t_list = tokenlist_create();
    tokenize(&t_list,&args);
    //tokenlist_print(t_list);
    
    generate_asm(&t_list,&args);
    tokenlist_free(&t_list);
    compilation_commands(&args);
    return 0;
}