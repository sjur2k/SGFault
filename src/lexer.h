#ifndef SGFAULT_LEXER
#define SGFAULT_LEXER

#define MAX_TOKEN_LEN 256

#include <stdlib.h>
#include "compiler_args.h"
typedef enum {
    _variable,
    _int_literal,
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

TokenList tokenize(CompilerArgs *args);
#endif
