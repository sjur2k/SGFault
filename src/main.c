#include <unistd.h>
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
    TokenList t_list = {0};
    LexerContext lexer_context = create_lexer_context(&t_list,args.source_file);
    tokenize(&lexer_context);   
    print_tokenlist(t_list);

    // ---- GRAMMAR ANALYSIS / GENERATION OF ABSTRACT SYNTAX TREE ----
    ASTList AST_list = {0};
    ParserContext parser_context = create_parser_context(&AST_list,t_list);
    parse(&parser_context);
    
    // ---- WALK AST -> ASSEMBLY ----
    generate_asm(&args);

    // ---- ASSEMBLE AND LINK ----
    build_binary(&args);

    // ---- CLEANUP / GARBAGE COLLECTION ----
    free_compiler_args(&args);
    free_tokenlist(&t_list);
    free_AST_list(&AST_list);

    return lexer_context.has_error || parser_context.has_error ? 1 : 0;
}