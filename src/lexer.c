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
    int i = 0;
    printf("Tokens:\n");
    while(t_list.data[i].type != _eof){
        printf("[%s] , ",t_list.data[i].value);
        i++;
    }
    printf("[EOF]");
}

void tokenlist_free(TokenList *t_list){
    free(t_list->data);
    t_list->data = NULL;
}

static void buf_push(char* buf, int *i, char c){
    if(*i >= MAX_TOKEN_LEN){
        fprintf(stderr,"Max token length exceeded");
        exit(1);
    }
    buf[*i] = c;
    (*i)++;
}
static bool is_word_delimiter(int c){
    if(c=='_') return false;
    return !isalnum(c);
}

void tokenize(TokenList* t_list, CompilerArgs* args){
    int symbol;
    char buf[MAX_TOKEN_LEN] = {0};
    int i = 0;
    while ((symbol = fgetc(args->in))!=EOF){
        //printf("%c",symbol);
        if(isalpha(symbol)||symbol=='_'){
            buf_push(buf,&i,symbol);
            while(!is_word_delimiter(symbol = fgetc(args->in))){
                buf_push(buf,&i,symbol);
            }
            if (str_eq(buf,"return")){
                tokenlist_push(t_list,(Token){_return,NULL});
            } else {
                buf_push(buf,&i,'\0');
                tokenlist_push(t_list, (Token){_variable,strdup(buf)});
            }
            // Restore symbol and reset buf
            ungetc(symbol,args->in);
            i=0;
            buf[0] = '\0';
        } else if (isdigit(symbol)){
            buf_push(buf,&i,symbol);
            while(isdigit(symbol = fgetc(args->in))){
                buf_push(buf,&i,symbol);
            }
            buf_push(buf,&i,'\0');
            tokenlist_push(t_list,(Token){_int_literal,strdup(buf)});
            // Restore symbol and reset buf
            ungetc(symbol,args->in);
            i=0;
            buf[0] = '\0';
        } else {
            switch (symbol){
                case '(':
                    tokenlist_push(t_list,(Token){_par_open,"("});
                    break;
                case ')':
                    tokenlist_push(t_list,(Token){_par_close,")"});
                    break;
                case '=':
                    tokenlist_push(t_list,(Token){_equal,"="});
                    break;
                case '+':
                    tokenlist_push(t_list,(Token){_operator,"+"});
                    break;
                case '-':
                    tokenlist_push(t_list,(Token){_operator,"-"});
                    break;
                case '/':
                    tokenlist_push(t_list,(Token){_operator,"/"});
                    break;
                case '*':
                    tokenlist_push(t_list,(Token){_operator,"*"});
                    break;
                case ';':
                    tokenlist_push(t_list,(Token){_semicolon,";"});
                    break;
                case '.':
                    tokenlist_push(t_list,(Token){_point,"."});
                    break;
                case ',':
                    tokenlist_push(t_list,(Token){_comma,","});
                    break;
            }
        }
    }
    tokenlist_push(t_list,(Token){_eof,"eof"});
    t_list->data = realloc(t_list->data,(t_list->size)*sizeof(Token));
    fclose(args->in);
}