#ifndef SGFAULT_LEXER
#define SGFAULT_LEXER

#include "stdio.h"

typedef enum {
    _return,
    _int,
    _semicolon
}TokenType;

typedef struct{
    TokenType type;
    char *value; // Optional. Nullptr if N/A
}Token;

typedef struct{
    TokenType* data;
    size_t size;
    size_t capacity
}TokenList;

TokenList tokenlist_create();
void tokenlist_push();
void tokenlist_free();

#endif
