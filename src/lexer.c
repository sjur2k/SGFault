#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "lexer.h"
#include "utils.h"

const char *token_type_names[_TOKEN_TYPE_COUNT] = {
    [_error]            = "Error",
    [_identifier]       = "Identifier",
    [_int_literal]      = "Int",
    [_str_literal]      = "Str",
    [_float_literal]    = "Float",
    [_equal]            = "Equal",
    [_point]            = "Point", // Maybe later
    [_comma]            = "Comma",
    [_semicolon]        = "Semicolon",
    [_par_open]         = "Par_open",
    [_par_close]        = "Par_close",
    [_bra_open]         = "Bra_open",
    [_bra_close]        = "Bra_close",
    [_sub]              = "Sub",
    [_add]              = "Add",
    [_mul]              = "Mul",
    [_div]              = "Div",
    [_return]           = "Return",
    [_eof]              = "Eof",
};

typedef struct{
    const char *word;
    TokenType type;
}Keyword;

typedef struct{
    char *data;
    int bufsize;
}Buffer;

static const Keyword keywords[] = {
    {"return", _return},
    { NULL   , _error },
};

// --------- Private function declarations -----------

// Tokenization functions
static void tokenize_identifier(LexerContext *context, int symbol);
static void tokenize_string(LexerContext *context, int symbol);
static void tokenize_number(LexerContext *context, int symbol);
static void tokenize_symbol(LexerContext *context, int symbol);

// Tokenlist functions
static void push_token(TokenList *t_list,Token t);

// Character buffer functions
static Buffer create_char_buffer(int bufsize);
static void free_char_buffer(Buffer *buf);
static int peek_char(LexerContext *context);
static bool push_char(Buffer *buf, int *i, char c, LexerContext *context);
static void push_string_char(Buffer *buf, int *i, char c);

// Helper functions
static bool is_word_delimiter(int c);
static TokenType keyword_type(const char *word);


// ------------ Public API -------------

/* Must be freed */
LexerContext create_lexer_context(TokenList *t_list, FILE *source_file){
    return (LexerContext){
        .t_list = t_list,
        .in = source_file,
        .line_number = 1,
        .has_error = false,
    };
}

// Main tokenization function:
void tokenize(LexerContext *context){
    int symbol;
    while ((symbol = fgetc(context->in))!=EOF){
        if (isalpha(symbol)||symbol=='_') {
            tokenize_identifier(context,symbol);
        }else if(symbol=='"'){
            tokenize_string(context,symbol);
        }else if(isdigit(symbol) || (symbol=='.' && isdigit(peek_char(context)))){
            tokenize_number(context,symbol);
        }else{
            tokenize_symbol(context,symbol);
        }
    }
    push_token(
        context->t_list,
        (Token){
            .type = _eof,
            .value.s = "EOF",
            .owned = false,
            .line_number = context->line_number,
        }
    );
    TokenList *t_list = context->t_list;
    t_list->data = realloc(t_list->data, t_list->size*sizeof(Token));
    if(!context->has_error) fprintf(stdout,"\033[1;32mNo syntactical errors detected!\033[0m\n");
}

void print_tokenlist(TokenList t_list){
    printf("\n\033[4mTokens - %d total:\033[0m\n",(int)t_list.size);
    for(size_t i = 0; i < t_list.size; i++){
        Token t = t_list.data[i];
        if (t.value.s == NULL)
            printf("\033[31mError\033[0m\n");
        else{
            char buf[MAX_TOKEN_LEN];
            switch (t.type){
                case _int_literal:   snprintf(buf,MAX_TOKEN_LEN,"%d",t.value.i); break;
                case _float_literal: snprintf(buf,MAX_TOKEN_LEN,"%f",t.value.f); break;            
                default:             snprintf(buf,MAX_TOKEN_LEN,"%s",t.value.s); break;
            }
            printf("Value: %-15s Type: %s\n", buf, token_type_names[t.type]);
        }
    }
    printf("\n");
}

void free_tokenlist(TokenList *t_list){
    for(size_t i = 0; i < t_list->size; i++){
        if(t_list->data[i].owned) free(t_list->data[i].value.s);
    }
    free(t_list->data);
}


// -------- Private function implementations -----------

// Tokenization functions:
static void tokenize_identifier(LexerContext *context, int symbol){
    int i = 0;
    TokenType word_type;
    Buffer buf = create_char_buffer(MAX_TOKEN_LEN);
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
    push_token(
        context->t_list,
        (Token){
            .type = word_type,
            .value.s = has_room ? str_dup(buf.data) : NULL,
            .owned = has_room,
            .line_number = context->line_number,
        }
    );
    free_char_buffer(&buf);
}

static void tokenize_string(LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    int opening_line = context->line_number;
    Buffer buf = create_char_buffer(64); // Resizes automatically. Is limited by size of input document.
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
    push_token(
        context->t_list,
        (Token){
            .type = ok ? _str_literal : _error,
            .value.s = ok ? str_dup(buf.data): NULL,
            .owned = ok,
            .line_number = context->line_number,
        }
    );
    free_char_buffer(&buf);
}

static void tokenize_number(LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    Buffer buf = create_char_buffer(MAX_TOKEN_LEN);
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

    Token t = {
        .type = has_room && ok ? number_type : _error,
        .owned = false,
        .line_number = context->line_number,
    };

    if(!ok){
        t.value.s = NULL; 
        push_token(context->t_list, t);
        free_char_buffer(&buf);
        return;
    }
    if (has_room) {
        buf.data[i]='\0';
        switch (number_type){
            case _int_literal: t.value.i = strtol(buf.data,NULL,10); break;
            case _float_literal: t.value.f = strtof(buf.data,NULL); break;
            default: break; // Compiler warning silencing
        }
    }    
    push_token(context->t_list,t);
    free_char_buffer(&buf);
}

static void tokenize_symbol(LexerContext *context, int symbol){
    bool ok = true;
    if(isspace(symbol)){
        if(symbol == '\n') context->line_number++;
        return;
    }
    TokenType symbol_type;
    switch (symbol){
        case '(': symbol_type = _par_open; break;
        case ')': symbol_type = _par_close; break;
        case '[': symbol_type = _bra_open; break;
        case ']': symbol_type = _bra_close; break;
        case '=': symbol_type = _equal; break;
        case '+': symbol_type = _add; break;
        case '-': symbol_type = _sub; break;
        case '/': symbol_type = _div; break;
        case '*': symbol_type = _mul; break;
        case ';': symbol_type = _semicolon; break;
        case '.': symbol_type = _point; break;
        case ',': symbol_type = _comma; break;
        default:
            fprintf(stderr,"\033[1;31mError on line %d:\033[0;0m %c is undefined in this context.\n",context->line_number,(unsigned char)symbol);
            context->has_error = true;
            ok = false;
            symbol_type = _error;
    }
    char value[2] = {symbol,'\0'};
    push_token(
        context->t_list,
        (Token){
            .type  = ok ? symbol_type : _error,
            .value.s = ok ? str_dup(value) : NULL,
            .owned = ok,
            .line_number = context->line_number,
        }
    );
}

// Tokenlist function:
static void push_token(TokenList *t_list, Token t){
    if(t_list->capacity == t_list->size){
        t_list->capacity = t_list->capacity == 0 ? 8 : t_list->capacity*2;
        t_list->data = realloc(t_list->data,(t_list->capacity)*sizeof(Token));
    }
    t_list->data[t_list->size++] = t;
}

// Character buffer functions:
static Buffer create_char_buffer(int bufsize){
    Buffer buf = {
        .data = safe_malloc(sizeof(char)*(bufsize+1)), // Explicit room for null-terminator
        .bufsize = bufsize,
    };
    return buf;
}

static void free_char_buffer(Buffer *buf){
    free(buf->data);
    buf->data = NULL;
}

static int peek_char(LexerContext *context){
    int c = fgetc(context->in);
    ungetc(c,context->in);
    return c;
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

// Helper functions:
static TokenType keyword_type(const char *word){
    for(int i = 0; keywords[i].word != NULL; i++){
        if(str_eq(word, keywords[i].word)) return keywords[i].type;
    }
    return _error;
}

static bool is_word_delimiter(int c){
    if(c==EOF) return true;
    return !isalnum((unsigned char)c) && c!='_';
}