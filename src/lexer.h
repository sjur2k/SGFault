#ifndef SGFAULT_LEXER_H
#define SGFAULT_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 256

typedef enum {
    tok_error,
    tok_identifier,
    tok_int_literal,
    tok_str_literal,
    tok_float_literal,
    tok_equal,
    tok_point,
    tok_comma,
    tok_semicolon,
    tok_par_open,
    tok_par_close,
    tok_sub,
    tok_add,
    tok_mul,
    tok_div,
    tok_return,
    tok_eof,
    TOK_TYPE_COUNT
}TokenType;

typedef struct{
    TokenType type;
    union{
        int i;
        float f;
        char *s; //Symbols, keywords, identifiers
    }value;
    bool owned;
    int line_number;
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

extern const char *token_type_names[TOK_TYPE_COUNT];

void print_tokenlist(TokenList t_list);
void free_tokenlist(TokenList *t_list);
void tokenize(LexerContext *context);
LexerContext create_lexer_context(TokenList *t_list, FILE *source_file);
#endif
