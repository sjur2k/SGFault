#ifndef SGFAULT_LEXER_H
#define SGFAULT_LEXER_H

#define MAX_TOKEN_LEN 256

#include <stdlib.h>
#include "compiler_args.h"
typedef enum {
    _variable,
    _int_literal,
    _float,
    _equal,
    _point,
    _comma,
    _semicolon,
    _par_open,
    _par_close,
    _operator,
    _return,
    _eof
}TokenType;
typedef struct{
    TokenType type;
    char *value; // Optional. Nullptr if N/A
}Token;

typedef struct{
    Token* data;
    size_t size;
    size_t capacity;
}TokenList;

TokenList tokenlist_create();
void tokenlist_push(TokenList *t_list, Token t);
void tokenlist_print(TokenList t_list);
void tokenlist_free(TokenList *t_list);

void tokenize(TokenList* t_list, CompilerArgs *args);
#endif
