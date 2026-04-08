#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "utils.h"

int line_number = 1;

/* Must be freed */
TokenList tokenlist_create(){
    TokenList t_list;
    t_list.size = 0;
    t_list.capacity = 1000; // Assuming most programs will be small (~200 lines)
    t_list.data = malloc(t_list.capacity*sizeof(Token));
    return t_list;
}

void tokenlist_push(TokenList *t_list,Token t){
    if(t_list->capacity == t_list->size){
        t_list->capacity *= 2;
        t_list->data = realloc(t_list->data,(t_list->capacity)*sizeof(Token));
    }
    t_list->data[t_list->size++] = t;
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

static void buf_push(char* buf, int *i, char c){
    if(*i >= MAX_TOKEN_LEN){
        fprintf(stderr,"Error on line %d: Max token length exceeded\n",line_number);
        exit(1);
    }
    buf[*i] = c;
    (*i)++;
}
static bool is_word_delimiter(int c){
    if(c=='_') return false;
    return !isalnum(c);
}

static int peek_next_symbol(CompilerArgs *args) {
    int c = fgetc(args->in);
    ungetc(c,args->in);
    return c;
}

static void tokenize_word(TokenList* t_list, CompilerArgs* args, char* buf, int symbol){
    int i = 0;
    TokenType word_type;
    buf_push(buf,&i,symbol);
    while(!is_word_delimiter(symbol = fgetc(args->in))){
        buf_push(buf,&i,symbol);
    }
    buf_push(buf,&i,'\0');
    if (str_eq(buf,"return")){
        word_type = _return;
    } else {
        word_type = _variable;
    }
    tokenlist_push(t_list,(Token){word_type,strdup(buf),true});
    
    // Restore symbol and reset buf
    ungetc(symbol,args->in);
}

static void tokenize_number(TokenList* t_list, CompilerArgs* args, char* buf, int symbol){
    int i = 0;
    TokenType number_type = _int_literal; //Assume integer
    if (symbol == '0' && isdigit(peek_next_symbol(args))){
        fprintf(stderr, "Error on line %d: Nonzero numbers cannot begin with 0\n",line_number);
        exit(1);
    }
    if (symbol == '.'){
        number_type = _float;
    }
    buf_push(buf,&i,symbol);
    while(isdigit(symbol = fgetc(args->in))){
        buf_push(buf,&i,symbol);
    }
    if (symbol=='.'){
        if(number_type == _float){
            fprintf(stderr, "Error on line %d: Floating point numbers can only have one decimal point.\n",line_number);
            exit(1);
        }
        number_type = _float;
        if(!isdigit(peek_next_symbol(args))){
            fprintf(stderr, "Error on line %d: Floating points without decimals are undefined.\n",line_number);
            exit(1);
        }
        buf_push(buf,&i,symbol);
        while(isdigit(symbol = fgetc(args->in))){
            buf_push(buf,&i,symbol);
        }
        if(symbol == '.'){
            fprintf(stderr, "Error on line %d: Floating point numbers can only have one decimal point.\n",line_number);
            exit(1);
        }
    } else if (isalpha(symbol) || symbol == '_'){
        fprintf(stderr, "Error on line %d: Variables cannot begin with a number.\n",line_number);
        exit(1);
    }
    buf_push(buf,&i,'\0');
    tokenlist_push(t_list,(Token){number_type,strdup(buf),true});
    // Restore symbol and reset buf
    ungetc(symbol,args->in);
}

static void tokenize_symbol(TokenList* t_list, int symbol){
    if(isspace(symbol)){
        if(symbol == '\n'){
            line_number++;
        }
        return;
    }
    switch (symbol){
        case '(':
            tokenlist_push(t_list,(Token){_par_open,"(",false});
            break;
        case ')':
            tokenlist_push(t_list,(Token){_par_close,")",false});
            break;
        case '=':
            tokenlist_push(t_list,(Token){_equal,"=",false});
            break;
        case '+':
            tokenlist_push(t_list,(Token){_operator,"+",false});
            break;
        case '-':
            tokenlist_push(t_list,(Token){_operator,"-",false});
            break;
        case '/':
            tokenlist_push(t_list,(Token){_operator,"/",false});
            break;
        case '*':
            tokenlist_push(t_list,(Token){_operator,"*",false});
            break;
        case ';':
            tokenlist_push(t_list,(Token){_semicolon,";",false});
            break;
        case '.':
            tokenlist_push(t_list,(Token){_point,".",false});
            break;
        case ',':
            tokenlist_push(t_list,(Token){_comma,",",false});
            break;
        default:
            fprintf(stderr,"Error on line %d: %c is a bad token\n",line_number,(unsigned char)symbol);
            exit(1);
    }
}

void tokenize(TokenList* t_list, CompilerArgs* args){
    int symbol;
    char buf[MAX_TOKEN_LEN] = {0};
    while ((symbol = fgetc(args->in))!=EOF){
        //printf("%c",symbol);
        if (isalpha(symbol)||symbol=='_') {
            tokenize_word(t_list,args,buf,symbol);
        }else if(isdigit(symbol) || (symbol=='.' && isdigit(peek_next_symbol(args)))){
            tokenize_number(t_list,args,buf,symbol);
        }else{
            tokenize_symbol(t_list,symbol);
        }
    }
    tokenlist_push(t_list,(Token){_eof,"EOF",false});
    t_list->data = realloc(t_list->data,(t_list->size)*sizeof(Token));
    fclose(args->in);
}