#include <stdbool.h>
#include "parser.h"
#include "utils.h"

typedef struct{
    float left;
    float right;
}BindingPower;

static const BindingPower binding_power[_TOKEN_TYPE_COUNT] = {
    [_eof]        = {0.0, 0.0},
    [_semicolon]  = {0.0, 0.0},
    [_return]     = {0.1, 0.0},
    [_add]        = {1.0, 1.1},
    [_sub]        = {1.0, 1.1},
    [_mul]        = {2.0, 2.1},
    [_div]        = {2.0, 2.1},
    [_par_open]   = {0.0, 3.0},
    [_par_close]  = {3.0, 0.0},
};

static Node *parse_expression(ParserContext *context, float min_bp);
static Node *create_node(Token token, Node* lhs, Node* rhs);
static bool is_atom(TokenType type);
static bool is_prefix(TokenType type);
static float bp_left(Token t);
static float bp_right(Token t);
static void push_AST(ASTList* AST_list, Node* AST);
static void free_AST(Node *root);

void parse(ParserContext *context){
    float min_binding_power = 0.0;
    while (context->t_list.data[context->token_index].type != _eof) {
        Node *tree = parse_expression(context, min_binding_power);
        print_AST(tree);
        if(tree!=NULL){
            printf("\n");
            context->line_number++;
        }
        Token semi = context->t_list.data[context->token_index++];
        if(semi.type!=_semicolon){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0m Expected a semicolon.\n",context->line_number);
            context->has_error = true;
            context->token_index--;
        }
        push_AST(context->AST_list,tree);
    }
}

ParserContext create_parser_context(ASTList* AST_list, TokenList t_list){
    return (ParserContext){
        .line_number = 0,
        .token_index = 0,
        .has_error = false,
        .t_list = t_list,
        .AST_list = AST_list,
    };
}

void print_AST(Node *root){
    if(root == NULL) return;
    print_AST(root->l);
    print_AST(root->r);
    printf("%s",root->token.value);
}

void free_AST_list(ASTList *AST_list){
    for (size_t i = 0; i < AST_list->size; i++){
        free_AST(AST_list->data[i]);
    }
    free(AST_list->data);
}

static Node *parse_expression(ParserContext *context, float min_bp){
    TokenList t_list = context->t_list;
    Token token = t_list.data[context->token_index++];
    if(token.type==_error){
        fprintf(stderr,"\033[1;32mError:\n\033[0mIll-defined token!\n");
        context->has_error = true;
        exit(1); // TODO: graceful error handling
    }
    
    struct Node *rhs, *lhs;
    if(is_atom(token.type)){
        lhs = create_node(token,NULL,NULL);
    } else if (is_prefix(token.type)){
        rhs = parse_expression(context, bp_right(token));
        lhs = create_node(token, NULL, rhs);
    }

    while(true){
        Token next_token = t_list.data[context->token_index];
        if (next_token.type == _eof || is_atom(next_token.type) || bp_left(next_token) <= min_bp){
            break;
        }
        context->token_index++;
        rhs = parse_expression(context, bp_right(next_token));
        lhs = create_node(next_token, lhs, rhs);
    }
    return lhs;
}

static bool is_atom(TokenType type){
    return (type == _str_literal   ||
            type == _float_literal ||
            type == _int_literal   ||
            type == _identifier);
}

static bool is_prefix(TokenType type){
    return (type == _return    ||
            type == _par_open);
}

static float bp_left(Token t){
    return binding_power[t.type].left;
}

static float bp_right(Token t){
    return binding_power[t.type].right;
}

static Node *create_node(Token token, Node *lhs, Node *rhs){
    Node *node = safe_malloc(sizeof(Node));
    node->token = token;
    node->l = lhs;
    node->r = rhs;
    return node;
}

static void push_AST(ASTList *AST_list, Node* AST){
    if(AST_list->capacity == AST_list->size){
        AST_list->capacity = AST_list->capacity == 0 ? 8 : 2*AST_list->capacity;
        AST_list->data = realloc(AST_list->data,AST_list->capacity*sizeof(Node *));
    }
    AST_list->data[AST_list->size++] = AST;
}

static void free_AST(Node *root){
    if(root == NULL) return;
    free_AST(root->l);
    free_AST(root->r);
    free(root);
}