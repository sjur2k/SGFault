#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "utils.h"

/* Must be freed */
TokenList tokenlist_create(){
    TokenList t_list;
    t_list.size = 0;
    t_list.capacity = 64;
    t_list.data = malloc(t_list.capacity*sizeof(Token));
    return t_list;
}

void tokenlist_print(TokenList t_list){
    printf("Tokens (%d total):\n",(int)t_list.size);
    for(size_t i = 0; i<t_list.size; i++){
        printf("[%s] ",t_list.data[i].value);
        if(i%8 == 7){
            printf("\n");
        }
    }
    printf("\n");
}

void tokenlist_free(TokenList *t_list){
    for(size_t i = 0; i < t_list->size; i++){
        if(t_list->data[i].owned){
            free(t_list->data[i].value);
        }
    }
    free(t_list->data);
    t_list->data = NULL;
}

static void tokenlist_push(TokenList *t_list,Token t){
    if(t_list->capacity == t_list->size){
        t_list->capacity *= 2;
        t_list->data = realloc(t_list->data,(t_list->capacity)*sizeof(Token));
    }
    t_list->data[t_list->size++] = t;
}

static void buf_push(char *buf, int *i, char c, LexerContext *lexer_args){
    if(*i >= MAX_TOKEN_LEN){
        fprintf(stderr,"\033[1;31mError on line %d:\033[0;0m Max token length exceeded\n",lexer_args->line_number);
        exit(1);
    }
    buf[*i] = c;
    (*i)++;
}

static bool is_word_delimiter(int c){
    if(c==EOF) return true;
    return !isalnum((unsigned char)c) && c!='_';
}

static int peek_char(LexerContext *lexer_args) {
    int c = fgetc(lexer_args->in);
    ungetc(c,lexer_args->in);
    return c;
}

static void tokenize_identifier(TokenList *t_list, LexerContext *lexer_args, int symbol){
    int i = 0;
    char buf[MAX_TOKEN_LEN] = {0};
    buf_push(buf,&i,symbol,lexer_args);
    TokenType word_type;
    while(!is_word_delimiter(symbol = fgetc(lexer_args->in))){
       buf_push(buf,&i,symbol,lexer_args);
    }
    ungetc(symbol, lexer_args->in);
    buf_push(buf,&i,'\0',lexer_args);
    if (str_eq(buf,"return")){
        word_type = _return;
    } else {
        word_type = _variable;
    }
    tokenlist_push(
        t_list,
        (Token){
            .type = word_type,
            .value = strdup(buf),
            .owned = true
        }
    );
}

static void tokenize_string(TokenList *t_list, LexerContext *lexer_args, int symbol){
    int i = 0;
    int opening_line = lexer_args->line_number;
    char buf[MAX_TOKEN_LEN] = {0};
    buf_push(buf, &i, symbol, lexer_args);
    while((symbol = fgetc(lexer_args->in))!='"'){
        if (symbol == EOF){
            fprintf(
                stderr, 
                "\033[1;31mError on line %d:\033[0;0m String literal beginning on line %d is missing closing quote\n",
                lexer_args->line_number, opening_line
            );
            exit(1);
        }
        if (symbol == '\n'){
            lexer_args->line_number++;
        }
        buf_push(buf, &i, symbol, lexer_args);
    }
    buf_push(buf, &i, symbol, lexer_args);
    buf_push(buf, &i, '\0', lexer_args);
    tokenlist_push(
        t_list,
        (Token){
            .type = _str_literal,
            .value = strdup(buf),
            .owned = true
        }
    );
}

static void tokenize_number(TokenList *t_list, LexerContext *lexer_args, int symbol){
    int i = 0;
    char buf[MAX_TOKEN_LEN] = {0};
    TokenType number_type = _int_literal; //Assume integer
    if (symbol == '0' && isdigit(peek_char(lexer_args))){
        fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Nonzero numbers cannot begin with 0\n",lexer_args->line_number);
        exit(1);
    }
    if (symbol == '.'){
        number_type = _float;
    }
    buf_push(buf,&i,symbol,lexer_args);
    while(isdigit(symbol = fgetc(lexer_args->in))){
        buf_push(buf,&i,symbol,lexer_args);
    }
    
    if (symbol=='.'){
        if(number_type == _float){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating point numbers can only have one decimal point.\n",lexer_args->line_number);
            exit(1);
        }
        number_type = _float;
        if(!isdigit(peek_char(lexer_args))){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating points without decimals are undefined.\n",lexer_args->line_number);
            exit(1);
        }
        buf_push(buf,&i,symbol,lexer_args);
        while(isdigit(symbol = fgetc(lexer_args->in))){
            buf_push(buf,&i,symbol,lexer_args);
        }
        if(symbol == '.'){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating point numbers can only have one decimal point.\n",lexer_args->line_number);
            exit(1);
        }
    } else if (isalpha(symbol) || symbol == '_'){
        fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Variables cannot begin with a number.\n",lexer_args->line_number);
        exit(1);
    }
    ungetc(symbol, lexer_args->in);
    buf_push(buf,&i,'\0',lexer_args);
    tokenlist_push(
        t_list,
        (Token){
            .type  = number_type,
            .value = strdup(buf),
            .owned = true
        }
    );
}

static void tokenize_symbol(TokenList *t_list, LexerContext *lexer_args, int symbol){
    if(isspace(symbol)){
        if(symbol == '\n'){
            lexer_args->line_number++;
        }
        return;
    }
    TokenType symbol_type;
    switch (symbol){
        case '(':
            symbol_type = _par_open;
            break;
        case ')':
            symbol_type = _par_close;
            break;
        case '=':
            symbol_type = _equal;
            break;
        case '+':
            symbol_type = _operator;
            break;
        case '-':
            symbol_type = _operator;
            break;
        case '/':
            symbol_type = _operator;
            break;
        case '*':
            symbol_type = _operator;
            break;
        case ';':
            symbol_type = _semicolon;
            break;
        case '.':
            symbol_type = _point;
            break;
        case ',':
            symbol_type = _comma;
            break;
        default:
            fprintf(stderr,"\033[1;31mError on line %d:\033[0;0m %c is undefined in this context.\n",lexer_args->line_number,(unsigned char)symbol);
            exit(1);
    }
    char value[2] = {symbol,'\0'};
    tokenlist_push(
        t_list,
        (Token){
            .type = symbol_type,
            .value = strdup(value),
            .owned = true
        }
    );
}

void tokenize(TokenList *t_list, LexerContext *lexer_args){
    int symbol;
    while ((symbol = fgetc(lexer_args->in))!=EOF){
        //printf("%c",symbol);
        if (isalpha(symbol)||symbol=='_') {
            tokenize_identifier(t_list,lexer_args,symbol);
        }else if(symbol=='"'){
            tokenize_string(t_list,lexer_args,symbol);
        }else if(isdigit(symbol) || (symbol=='.' && isdigit(peek_char(lexer_args)))){
            tokenize_number(t_list,lexer_args,symbol);
        }else{
            tokenize_symbol(t_list,lexer_args,symbol);
        }
    }
    tokenlist_push(
        t_list,
        (Token){
            .type = _eof,
            .value = "EOF",
            .owned = false
        }
    );
    t_list->data = realloc(t_list->data,(t_list->size)*sizeof(Token));
}