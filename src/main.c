#include <unistd.h>
#include "compiler_args.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "asm_gen.h"

int main(int argc, char *argv[]){
    CompilerArgs args = parse_args(argc,argv);
    
    // ---- LEXICAL ANALYSIS ----
    TokenList t_list = {0};
    LexerContext lexer_context = create_lexer_context(&t_list, args.source_name, args.source_file);
    tokenize(&lexer_context);   
    //print_tokenlist(t_list);

    // ---- GRAMMAR ANALYSIS / GENERATION OF ABSTRACT SYNTAX TREE ----
    ASTList AST_list = {0};
    ParserContext parser_context = create_parser_context(&AST_list, args.source_name, t_list);
    parse(&parser_context);
    
    // ---- WALK AST -> ASSEMBLY ----
    AsmContext asm_context = create_asm_context(args.out, AST_list);
    generate_asm(&asm_context);

    // ---- ASSEMBLE AND LINK ----
    build_binary(&args);

    // ---- CLEANUP / GARBAGE COLLECTION ----
    free_compiler_args(&args);
    free_tokenlist(&t_list);
    free_AST_list(&AST_list);

    return lexer_context.has_error || parser_context.has_error ? 1 : 0;
}