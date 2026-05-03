#ifndef SGFAULT_PARSER_H
#define SGFAULT_PARSER_H

#include <stdbool.h>
#include "lexer.h"
#include "ast.h"

typedef struct{
    int token_index;
    bool has_error;
    char *src_name;
    TokenList t_list;
    ASTList *AST_list;
}ParserContext;

ParserContext create_parser_context(ASTList *AST_list, char* source_name, TokenList t_list);
void parse(ParserContext *context);
void free_AST_list(ASTList *AST_list);
#endif