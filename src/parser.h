#ifndef SGFAULT_PARSER_H
#define SGFAULT_PARSER_H

#include <stdbool.h>
#include "lexer.h"

typedef enum {
    op_add,
    op_sub,
    op_mul,
    op_div,
    op_eof,
    op_NUM
}OperatorType;

typedef struct Node {
    Token token;
    struct Node *l;
    struct Node *r;
}Node;

typedef struct{
    Node **data;
    size_t size;
    size_t capacity;
}ASTList;

typedef struct{
    int line_number;
    int token_index;
    bool has_error;
    TokenList t_list;
    ASTList *AST_list;
}ParserContext;
ParserContext create_parser_context(ASTList *AST_list, TokenList t_list);
void parse(ParserContext *context);
void print_AST(Node *root);
void free_AST_list(ASTList *AST_list);
#endif