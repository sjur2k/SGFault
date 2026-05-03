#ifndef SGFAULT_LEXER_H
#define SGFAULT_LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TOKEN_LEN 256

typedef enum {
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_STR_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_EQUAL,
    TOKEN_POINT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_PAR_OPEN,
    TOKEN_PAR_CLOSE,
    TOKEN_BRA_OPEN,
    TOKEN_BRA_CLOSE,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_RETURN,
    TOKEN_EOF,
    TOKEN_TYPE_COUNT
}TokenType;

typedef struct{
    TokenType type;
    union{
        int i;
        float f;
        char *s; //Symbols, keywords, identifiers
    }value;
    bool owned;
    int line, col;
}Token;

typedef struct{
    Token *data;
    size_t size;
    size_t capacity;
}TokenList;

typedef struct{
    TokenList *t_list;
    FILE *in;
    char *src_name;
    int line, col;
    bool has_error;
} LexerContext;

extern const char *token_type_names[TOKEN_TYPE_COUNT];

void print_tokenlist(TokenList t_list);
void free_tokenlist(TokenList *t_list);
void tokenize(LexerContext *context);
LexerContext create_lexer_context(TokenList *t_list, char *source_name, FILE *source_file);
#endif
