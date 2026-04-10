#include "compiler_args.h"
#include "lexer.h"
#include "parser.h"

void generate_asm(CompilerArgs *args){
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
    LexerContext context = {.in = args.in, .line_number = 1, .has_error = false};

    TokenList t_list = tokenlist_create();
    tokenize(&t_list,&context);
    
    //parse(&t_list, &args);
    tokenlist_print(t_list);
    generate_asm(&args);
    compilation_commands(&args);

    // Garbage collection / Cleanup
    compiler_args_free(&args);
    tokenlist_free(&t_list);

    return context.has_error ? 1 : 0;
}