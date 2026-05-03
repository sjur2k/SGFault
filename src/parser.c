#include <stdbool.h>
#include "parser.h"
#include "utils.h"

typedef struct{
    float left;
    float right;
}BindingPower;

static const BindingPower binding_power[TOKEN_TYPE_COUNT] = {
    [TOKEN_EOF]        = {0.0, 0.0},
    [TOKEN_SEMICOLON]  = {0.0, 0.0},
    [TOKEN_COMMA]      = {2.1, 2.0},
    [TOKEN_RETURN]     = {0.1, 0.0},
    [TOKEN_EQUAL]      = {1.0, 1.1},
    [TOKEN_PLUS]       = {3.0, 3.1},
    [TOKEN_MINUS]      = {3.0, 3.1},
    [TOKEN_MUL]        = {4.0, 4.1},
    [TOKEN_DIV]        = {4.0, 4.1},
    [TOKEN_BRA_OPEN]   = {5.0, 0.0},
    [TOKEN_BRA_CLOSE]  = {0.0, 5.0},
    [TOKEN_PAR_OPEN]   = {5.0, 0.0},
    [TOKEN_PAR_CLOSE]  = {0.0, 5.0},
};

// -------- Private functions declarations ----------

// Recursive parse function:
static Node *parse_expression(ParserContext *context, float min_bp);

// Parsing helper functions:
static Node *create_node(Token token, Node* lhs, Node* rhs);
static bool is_atom(TokenType type);
static bool is_prefix(TokenType type);
static float bp_left(Token t);
static float bp_right(Token t);

// Error logging
static void parser_error(Token token, ParserContext context, char *msg);
// ------------ Public API ---------------

// Main parsing loop:
void parse(ParserContext *context){
    float min_binding_power = 0.0;
    while (context->t_list.data[context->token_index].type != TOKEN_EOF) {
        Node *tree = parse_expression(context, min_binding_power);
        if(tree){
            print_AST(tree);
            printf("\n");
        }
        // If missing semi, go to next semi and continue parsing.
        Token semi = context->t_list.data[context->token_index++];
        while(semi.type!=TOKEN_SEMICOLON){
            fprintf(stderr, "\033[1;31mError on line %d:\033[0m Expected a semicolon.\n",semi.line);
            context->has_error = true;
            semi = context->t_list.data[context->token_index++];
        }
        push_AST(context->AST_list,tree);
    }
}

ParserContext create_parser_context(ASTList* AST_list, char *source_name, TokenList t_list){
    return (ParserContext){
        .token_index = 0,
        .src_name = source_name,
        .has_error = false,
        .t_list = t_list,
        .AST_list = AST_list,
    };
}

// --------- Private functions implementations -----------

// Recursive parse function
static Node *parse_expression(ParserContext *context, float min_bp){
    TokenList t_list = context->t_list;
    Token token = t_list.data[context->token_index++];
    if(token.type==TOKEN_ERROR){
        fprintf(stderr,"\033[1;31mError:\n\033[0mIll-defined token!\n");
        context->has_error = true;
        exit(1); // TODO: graceful error handling
    }
    
    struct Node *rhs, *lhs;
    if(is_atom(token.type)){
        switch (token.type){
        case TOKEN_INT_LITERAL:
            lhs = create_int_lit(token);
            break;
        case TOKEN_FLOAT_LITERAL:
            lhs = create_float_lit(token);
            break;
        case TOKEN_IDENTIFIER:
            lhs = create_ident(token);
            break;
        default:
            parser_error(token,*context,"Expected atomic token.");
            break;
        }
    } else if (is_prefix(token.type)){
        if(token.type==TOKEN_PAR_OPEN){
            lhs = parse_expression(context, bp_right(token));
            Token close = context->t_list.data[context->token_index++];
            if(close.type != TOKEN_PAR_CLOSE){
                fprintf(stderr,"\033[1;31mError on line %d:\n\033[0m Expected closing parenthesis.\n",close.line);
                context->has_error = true;
                free_AST(lhs);
                return NULL;
            }
        } else if(token.type==TOKEN_BRA_OPEN){
            lhs = parse_expression(context,bp_right(token));
            Token close = context->t_list.data[context->token_index++];
            if(close.type != TOKEN_BRA_CLOSE){
                fprintf(stderr,"\033[1;31mError on line %d:\n\033[0m Expected closing bracket.\n",close.line);
                context->has_error = true;
                free_AST(lhs);
                return NULL;
            }
            lhs = create_node(token,NULL,lhs);
        } else {
        rhs = parse_expression(context, bp_right(token));
        lhs = create_node(token, NULL, rhs);
        }
    }

    while(true){
        Token next_token = t_list.data[context->token_index];
        if (next_token.type == TOKEN_EOF || is_atom(next_token.type) || bp_left(next_token) <= min_bp){
            break;
        }
        context->token_index++;
        rhs = parse_expression(context, bp_right(next_token));
        lhs = create_node(next_token, lhs, rhs);
    }
    return lhs;
}

// Parsing helper functions:

static bool is_atom(TokenType type){
    return (type == TOKEN_STR_LITERAL   ||
            type == TOKEN_FLOAT_LITERAL ||
            type == TOKEN_INT_LITERAL   ||
            type == TOKEN_IDENTIFIER);
}

static bool is_prefix(TokenType type){
    return (type == TOKEN_RETURN   ||
            type == TOKEN_PAR_OPEN ||
            type == TOKEN_BRA_OPEN);
}

static float bp_left(Token t){
    return binding_power[t.type].left;
}

static float bp_right(Token t){
    return binding_power[t.type].right;
}

static void parser_error(Token token, ParserContext context, char* msg){
    fprintf(stderr, "%s:%d:%d: \033[1;31mError\n\033[0m: %s\n",context.src_name,token.line,token.col,msg);
}