#include "compiler_args.h"
#include "lexer.h"

void generate_asm(CompilerArgs* args){
    //PLACEHOLDER CODE:
    fprintf(args->out,"global _start\n\n");
    fprintf(args->out,"_start:\n");
    fprintf(args->out,"\tmov rdi, 0\n");
    fprintf(args->out,"\tmov rax, 60\n");
    fprintf(args->out,"\tsyscall");
    fclose(args->out);
    args->out = NULL;
}

int main(int argc, char *argv[]){
    CompilerArgs args = parse_args(argc,argv);
    
    TokenList t_list = tokenize(&args);
    int i = 0;
    printf("Tokens:\n%c",'[');
    while(t_list.data[i].type != _eof){
        printf("%s ",t_list.data[i].value);
        i++;
    }
    printf("]");

    /* 
    printf("%s ,",t_list.data[0].value);
    printf("%s,",t_list.data[1].value);
    printf("%s,",t_list.data[2].value);
    printf("%s]",t_list.data[3].value); */
    
    free(t_list.data);
    //parse_tokens();
    generate_asm(&args);
    compilation_commands(&args);
    return 0;
}