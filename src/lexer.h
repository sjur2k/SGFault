#ifndef SGFAULT_LEXER_H
#define SGFAULT_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 256

typedef enum {
    _error,
    _identifier,
    _int_literal,
    _str_literal,
    _float_literal,
    _equal,
    _point,
    _comma,
    _semicolon,
    _par_open,
    _par_close,
    _sub,
    _add,
    _mul,
    _div,
    _return,
    _eof,
    _TOKEN_TYPE_COUNT
}TokenType;

typedef struct{
    TokenType type;
    char *value;
    bool owned;
}Token;

typedef struct{
    Token *data;
    size_t size;
    size_t capacity;
}TokenList;

typedef struct{
    TokenList *t_list;
    FILE *in;
    int line_number;
    bool has_error;
} LexerContext;

extern const char *token_type_names[_TOKEN_TYPE_COUNT];

TokenList tokenlist_create(void);
void tokenlist_print(TokenList t_list);
void tokenlist_free(TokenList *t_list);
void tokenize(LexerContext *context);
LexerContext lexer_context_create(TokenList *t_list, FILE *source_file);
#endif
