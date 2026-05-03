#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "lexer.h"
#include "utils.h"

#define TAB_WIDTH 4

const char *token_type_names[TOKEN_TYPE_COUNT] = {
    [TOKEN_ERROR]            = "Error",
    [TOKEN_IDENTIFIER]       = "Identifier",
    [TOKEN_INT_LITERAL]      = "Int",
    [TOKEN_STR_LITERAL]      = "Str",
    [TOKEN_FLOAT_LITERAL]    = "Float",
    [TOKEN_EQUAL]            = "Equal",
    [TOKEN_POINT]            = "Point", // Maybe later
    [TOKEN_COMMA]            = "Comma",
    [TOKEN_SEMICOLON]        = "Semicolon",
    [TOKEN_PAR_OPEN]         = "Par_open",
    [TOKEN_PAR_CLOSE]        = "Par_close",
    [TOKEN_BRA_OPEN]         = "Bra_open",
    [TOKEN_BRA_CLOSE]        = "Bra_close",
    [TOKEN_MINUS]            = "Minus",
    [TOKEN_PLUS]             = "Plus",
    [TOKEN_MUL]              = "Mul",
    [TOKEN_DIV]              = "Div",
    [TOKEN_RETURN]           = "Return",
    [TOKEN_EOF]              = "Eof",
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
    {"return", TOKEN_RETURN},
    { NULL   , TOKEN_ERROR },
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

// Error logging
static void lexer_error(LexerContext context, char* msg);
// ------------ Public API -------------

/* Must be freed */
LexerContext create_lexer_context(TokenList *t_list, char *source_name, FILE *source_file){
    return (LexerContext){
        .t_list = t_list,
        .in = source_file,
        .src_name = source_name,
        .line = 1,
        .col = 0, // "Initializes" to 1 when 1st symbol is read.
        .has_error = false,
    };
}

// Main tokenization function:
void tokenize(LexerContext *context){
    int symbol;
    while ((symbol = fgetc(context->in))!=EOF){
        context->col++;
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
            .type = TOKEN_EOF,
            .value.s = "EOF",
            .owned = false,
            .line = context->line,
            .col = context->col,
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
                case TOKEN_INT_LITERAL:   snprintf(buf,MAX_TOKEN_LEN,"%d",t.value.i); break;
                case TOKEN_FLOAT_LITERAL: snprintf(buf,MAX_TOKEN_LEN,"%f",t.value.f); break;            
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

static void lexer_error(LexerContext context, char* msg){
    fprintf(stderr, "%s:%d:%d: \033[31mError\033[0m: %s\n",
        context.src_name,
        context.line,
        context.col,
        msg
    );
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
            context->col++;
            has_room = push_char(&buf, &i, symbol, context);
        }
    }
    ungetc(symbol, context->in);
    
    if(has_room) {
        buf.data[i]='\0';
        TokenType kw = keyword_type(buf.data);
        word_type = kw == TOKEN_ERROR ? TOKEN_IDENTIFIER : kw;
    } else {
        word_type = TOKEN_ERROR;
    }
    push_token(
        context->t_list,
        (Token){
            .type = word_type,
            .value.s = has_room ? str_dup(buf.data) : NULL,
            .owned = has_room,
            .line = context->line,
            .col = context->col,
        }
    );
    free_char_buffer(&buf);
}

static void tokenize_string(LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    Buffer buf = create_char_buffer(64); // Resizes automatically. Is limited by size of input document.
    while((symbol = fgetc(context->in))!='"'){
        context->col++;
        if (symbol == EOF){
            lexer_error(*context,"String literal is missing closing quote");
            context->has_error = true;
            ok = false;
            break;
        }
        if (symbol == '\n'){
            context->line++;
            context->col = 1;
        }
        push_string_char(&buf, &i, symbol);
    }
    if(ok) push_string_char(&buf, &i, '\0');
    push_token(
        context->t_list,
        (Token){
            .type = ok ? TOKEN_STR_LITERAL : TOKEN_ERROR,
            .value.s = ok ? str_dup(buf.data): NULL,
            .owned = ok,
            .line = context->line,
            .col = context->col,
        }
    );
    free_char_buffer(&buf);
}

static void tokenize_number(LexerContext *context, int symbol){
    int i = 0;
    bool ok = true;
    Buffer buf = create_char_buffer(MAX_TOKEN_LEN);
    TokenType number_type = TOKEN_INT_LITERAL; //Assume integer
    if (symbol == '0' && isdigit(peek_char(context))){
        lexer_error(*context,"Nonzero numbers cannot begin with 0.");
        context->has_error = true;
        ok = false;
    }
    if (symbol == '.') number_type = TOKEN_FLOAT_LITERAL;
    bool has_room = push_char(&buf,&i,symbol,context);
    while(isdigit(symbol = fgetc(context->in))){
        if(has_room){
            context->col++;
            has_room = push_char(&buf,&i,symbol,context);
        }
    }
    
    if (symbol=='.'){
        if(has_room){
            context->col++;
            has_room = push_char(&buf, &i, symbol, context);
        } 
        if(number_type == TOKEN_FLOAT_LITERAL){
            lexer_error(*context,"Floating point numbers can only have one decimal point.");
            context->has_error = true;
            ok = false;
        }
        number_type = TOKEN_FLOAT_LITERAL;
        if(!isdigit(peek_char(context))){
            lexer_error(*context,"Floating points without decimals are undefined.");
            context->has_error = true;
            ok = false;
        }
        while(isdigit(symbol = fgetc(context->in))){
            if(has_room){
                context->col++;
                has_room = push_char(&buf,&i,symbol,context);
            }
        }
        context->col++;
        if(symbol == '.'){
            lexer_error(*context,"Floating point numbers can only have one decimal point.");
            context->has_error = true;
            ok = false;
        }
    } else if (isalpha(symbol) || symbol == '_'){
        lexer_error(*context,"Identifiers cannot begin with a number.");
        context->has_error = true;
        ok = false;
    }
    ungetc(symbol, context->in);

    Token t = {
        .type = has_room && ok ? number_type : TOKEN_ERROR,
        .owned = false,
        .line = context->line,
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
            case TOKEN_INT_LITERAL: t.value.i = strtol(buf.data,NULL,10); break;
            case TOKEN_FLOAT_LITERAL: t.value.f = strtof(buf.data,NULL); break;
            default: break; // Compiler warning silencing
        }
    }    
    push_token(context->t_list,t);
    free_char_buffer(&buf);
}

static void tokenize_symbol(LexerContext *context, int symbol){
    bool ok = true;
    if(isspace((unsigned char)symbol)){
        switch (symbol){
            case ' ':
                context->col++;
                break;
            case '\n':
                context->line++;
                context->col = 1;
                break;
            case '\t':
                context->col += TAB_WIDTH - ((context->col-1) % TAB_WIDTH);
                break;
            default:
                break;
            return;
        }
    }
    TokenType symbol_type;
    switch (symbol){
        case '(': symbol_type = TOKEN_PAR_OPEN; break;
        case ')': symbol_type = TOKEN_PAR_CLOSE; break;
        case '[': symbol_type = TOKEN_BRA_OPEN; break;
        case ']': symbol_type = TOKEN_BRA_CLOSE; break;
        case '=': symbol_type = TOKEN_EQUAL; break;
        case '+': symbol_type = TOKEN_PLUS; break;
        case '-': symbol_type = TOKEN_MINUS; break;
        case '/': symbol_type = TOKEN_DIV; break;
        case '*': symbol_type = TOKEN_MUL; break;
        case ';': symbol_type = TOKEN_SEMICOLON; break;
        case '.': symbol_type = TOKEN_POINT; break;
        case ',': symbol_type = TOKEN_COMMA; break;
        default:
            lexer_error(*context,"Symbol is undefined in this context:");
            fprintf(stderr,"\033[1;31m%c\033[0;0m\n",(unsigned char)symbol);
            context->has_error = true;
            ok = false;
            symbol_type = TOKEN_ERROR;
    }
    char value[2] = {symbol,'\0'};
    push_token(
        context->t_list,
        (Token){
            .type  = ok ? symbol_type : TOKEN_ERROR,
            .value.s = ok ? str_dup(value) : NULL,
            .owned = ok,
            .line = context->line,
            .col = context->col,
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
        lexer_error(*context, "Max token length exceeded.");
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
    return TOKEN_ERROR;
}

static bool is_word_delimiter(int c){
    if(c==EOF) return true;
    return !isalnum((unsigned char)c) && c!='_';
}