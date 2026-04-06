#include "lexer.h"
#include "compiler_args.h"

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
        t_list->data = realloc(t_list->data,t_list->capacity);
    }
    t_list->data[t_list->size++] = t;
}

TokenList tokenizer(CompilerArgs* args){

}