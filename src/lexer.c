#include "lexer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include "utils.h"

const char *token_type_names[_TOKEN_TYPE_COUNT] = {
    [_error]            = "Error",
    [_identifier]       = "Identifier",
    [_int_literal]      = "Int",
    [_str_literal]      = "Str",
    [_float_literal]    = "Float",
    [_equal]            = "Equal",
    [_point]            = "Point",
    [_comma]            = "Comma",
    [_semicolon]        = "Semicolon",
    [_par_open]         = "Par_open",
    [_par_close]        = "Par_close",
    [_sub]              = "Sub",
    [_add]              = "Add",
    [_mul]              = "Mul",
    [_div]              = "Div",
    [_return]           = "Return",
    [_eof]              = "Eof"
};

// ----------- TOKENLISTS(-BUFFERS) ----------------

/* Must be freed */
TokenList tokenlist_create(){
    TokenList t_list;
    t_list.size = 0;
    t_list.capacity = 64;
    t_list.data = malloc(t_list.capacity*sizeof(Token));
    return t_list;
}
LexerContext lexer_context_create(FILE *source_file){
    return (LexerContext){
        .in = source_file,
        .line_number = 1,
        .has_error = false
    };
}

void tokenlist_print(TokenList t_list){
    printf("\n\033[4mTokens - %d total:\033[0m\n",(int)t_list.size);
    for(size_t i = 0; i<t_list.size; i++){
        if (t_list.data[i].value == NULL)
            printf("\033[31mError\033[0m\n");
        else
            printf("Value: %-15sType: %s\n",t_list.data[i].value, token_type_names[t_list.data[i].type]);
    }
    printf("\n");
}

static void tokenlist_push(TokenList *t_list,Token t){
    if(t_list->capacity == t_list->size){
        t_list->capacity *= 2;
        t_list->data = realloc(t_list->data,(t_list->capacity)*sizeof(Token));
    }
    t_list->data[t_list->size++] = t;
}

void tokenlist_free(TokenList *t_list){
    for(size_t i = 0; i < t_list->size; i++){
        if(t_list->data[i].owned) free(t_list->data[i].value);
    }
    free(t_list->data);
    t_list->data = NULL;
}

// ------------- KEYWORDS ---------------

typedef struct{
    const char *word;
    TokenType type;
}Keyword;

static const Keyword keywords[] = {
    {"return", _return},
    { NULL   , _error },
};

static TokenType keyword_type(const char *word){
    for(int i = 0; keywords[i].word != NULL; i++){
        if(str_eq(word, keywords[i].word)) return keywords[i].type;
    }
    return _error;
}

// ------------- BUFFERS --------------------------------

typedef struct{
    char *data;
    int bufsize;
}Buffer;

static Buffer char_buffer_create(int bufsize){
    Buffer buf = {
        .data = malloc(sizeof(char)*(bufsize+1)), // Explicit room for null-terminator
        .bufsize = bufsize,
    };
    return buf;
}

static void char_buffer_free(Buffer *buf){
    free(buf->data);
    buf->data = NULL;
}


static bool push_char(Buffer *buf, int *i, char c, LexerContext *context){
    if(*i >= MAX_TOKEN_LEN){
        fprintf(stderr,"\033[1;31mError on line %d:\033[0;0m Max token length (%d) exceeded\n",context->line_number,MAX_TOKEN_LEN);
        context->has_error = true;
        return false;
    }
    buf->data[*i] = c;
    (*i)++;
    return true;
}

static void push_string_char(Buffer *buf, int *i, char c){
    if(*i >= buf->bufsize){
        buf->bufsize *= 2;
        buf->data = realloc(buf->data, sizeof(char)*buf->bufsize);
    }
    buf->data[*i] = c;
    (*i)++;
}

// ---------- HELPERS ------------

static bool is_word_delimiter(int c){
    if(c==EOF) return true;
    return !isalnum((unsigned char)c) && c!='_';
}

static int peek_char(LexerContext *context) {
    int c = fgetc(context->in);
    ungetc(c,context->in);
    return c;
}

// -------------- TOKENIZATION -----------------

static void tokenize_identifier(TokenList *t_list, LexerContext *context, int symbol){
    int i = 0;
    TokenType word_type;
    Buffer buf = char_buffer_create(MAX_TOKEN_LEN);
    bool has_room = push_char(&buf,&i,symbol,context);
    while(!is_word_delimiter(symbol = fgetc(context->in))){
        if(has_room){
            has_room = push_char(&buf, &i, symbol, context);
        }
    }
    ungetc(symbol, context->in);
    
    if(has_room) {
        buf.data[i]='\0';
        TokenType kw = keyword_type(buf.data);
        word_type = kw == _error ? _identifier : kw;
    } else {
        word_type = _error;
    }
    tokenlist_push(
        t_list,
        (Token){
            .type = word_type,
            .value = has_room ? strdup(buf.data) : NULL,
            .owned = has_room
        }
    );
    char_buffer_free(&buf);
}

static void tokenize_string(TokenList *t_list, LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    int opening_line = context->line_number;
    Buffer buf = char_buffer_create(64); // Resizes automatically. Is limited by size of input document.
    while((symbol = fgetc(context->in))!='"'){
        if (symbol == EOF){
            fprintf(
                stderr, 
                "\033[1;31mError on line %d:\033[0;0m String literal beginning on line %d is missing closing quote\n",
                context->line_number, opening_line
            );
            context->has_error = true;
            ok = false;
            break;
        }
        if (symbol == '\n') context->line_number++;
        push_string_char(&buf, &i, symbol);
    }
    if(ok) push_string_char(&buf, &i, '\0');
    tokenlist_push(
        t_list,
        (Token){
            .type = ok ? _str_literal : _error,
            .value = ok ? strdup(buf.data): NULL,
            .owned = ok
        }
    );
    char_buffer_free(&buf);
}

static void tokenize_number(TokenList *t_list, LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    Buffer buf = char_buffer_create(MAX_TOKEN_LEN);
    TokenType number_type = _int_literal; //Assume integer
    if (symbol == '0' && isdigit(peek_char(context))){
        fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Nonzero numbers cannot begin with 0\n",context->line_number);
        context->has_error = true;
        ok = false;
    }
    if (symbol == '.') number_type = _float_literal;
    bool has_room = push_char(&buf,&i,symbol,context);
    while(isdigit(symbol = fgetc(context->in))){
        if(has_room) has_room = push_char(&buf,&i,symbol,context);
    }
    
    if (symbol=='.'){
        if(has_room) has_room = push_char(&buf, &i, symbol,context);
        if(number_type == _float_literal){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating point numbers can only have one decimal point.\n",context->line_number);
            context->has_error = true;
            ok = false;
        }
        number_type = _float_literal;
        if(!isdigit(peek_char(context))){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating points without decimals are undefined.\n",context->line_number);
            context->has_error = true;
            ok = false;
        }
        while(isdigit(symbol = fgetc(context->in))){
            if(has_room) has_room = push_char(&buf,&i,symbol,context);
        }
        if(symbol == '.'){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Floating point numbers can only have one decimal point.\n",context->line_number);
            context->has_error = true;
            ok = false;
        }
    } else if (isalpha(symbol) || symbol == '_'){
        fprintf(stderr, "\033[1;31mError on line %d:\033[0;0m Identifiers cannot begin with a number.\n",context->line_number);
        context->has_error = true;
        ok = false;
    }
    ungetc(symbol, context->in);
    if (has_room) buf.data[i]='\0';
    tokenlist_push(
            t_list,
            (Token){
                .type  = has_room && ok ? number_type : _error,
                .value = has_room && ok ? strdup(buf.data): NULL,
                .owned = has_room && ok
            }
        );
    char_buffer_free(&buf);
}

static void tokenize_symbol(TokenList *t_list, LexerContext *context, int symbol){
    bool ok = true;
    if(isspace(symbol)){
        if(symbol == '\n') context->line_number++;
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
            symbol_type = _add;
            break;
        case '-':
            symbol_type = _sub;
            break;
        case '/':
            symbol_type = _div;
            break;
        case '*':
            symbol_type = _mul;
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
            fprintf(stderr,"\033[1;31mError on line %d:\033[0;0m %c is undefined in this context.\n",context->line_number,(unsigned char)symbol);
            context->has_error = true;
            ok = false;
            symbol_type = _error;
    }
    char value[2] = {symbol,'\0'};
    tokenlist_push(
        t_list,
        (Token){
            .type  = ok ? symbol_type : _error,
            .value = ok ? strdup(value) : NULL,
            .owned = ok
        }
    );
}

void tokenize(TokenList *t_list, LexerContext *context){
    int symbol;
    while ((symbol = fgetc(context->in))!=EOF){
        if (isalpha(symbol)||symbol=='_') {
            tokenize_identifier(t_list,context,symbol);
        }else if(symbol=='"'){
            tokenize_string(t_list,context,symbol);
        }else if(isdigit(symbol) || (symbol=='.' && isdigit(peek_char(context)))){
            tokenize_number(t_list,context,symbol);
        }else{
            tokenize_symbol(t_list,context,symbol);
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
    if(!context->has_error) fprintf(stdout,"\033[1;32mNo syntactical errors detected!\033[0m\n");
}