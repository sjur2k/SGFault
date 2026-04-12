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
    
    // ---- LEXICAL ANALYSIS ----
    LexerContext lexer_context = lexer_context_create(args.source_file);
    TokenList t_list = tokenlist_create();
    tokenize(&t_list,&lexer_context);   
    tokenlist_print(t_list);

    // ---- GRAMMAR ANALYSIS / ABSTRACT SYNTAX TREE (AST) GENERATION ----
    ParserContext parser_context = parser_context_create();
    parse(&t_list, &parser_context);

    // ---- WALK AST -> ASSEMBLY ----
    generate_asm(&args);

    // ---- ASSEMBLE AND LINK ----
    build_binary(&args);

    // ---- CLEANUP / GARBAGE COLLECTION ----
    compiler_args_free(&args);
    tokenlist_free(&t_list);
    
    return lexer_context.has_error || parser_context.has_error ? 1 : 0;
}