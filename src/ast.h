#ifndef SGFAULT_AST_H
#define SGFAULT_AST_H

#include <stdlib.h>
#include "lexer.h"

typedef enum {
    // Literals
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_IDENTIFIER,

    // Expressions
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_ASSIGN,
    NODE_TYPE_REF,

    // Statements
    NODE_VAR_DECL,
    NODE_BLOCK,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_EXPR_STMT,
    
    // Top level
    NODE_PROGRAM,

    NODE_TYPE_COUNT
}NodeType;

typedef struct Node {
    NodeType type;
    int line,col;
    NodeType* result_type;
    union {
        struct { long value; } int_lit;
        struct { double value; } float_lit;
        struct { char* name; } ident;
        struct { 
            TokenType op;
            struct Node *l,*r;
        } bin_op;
        struct {
            TokenType op;
            struct Node *operand;
        } un_op;
        struct {
            TokenType op;
            struct Node *target;
            struct Node *value;
        } assign;
        struct {
            char *name;
        } type_ref;
        struct {
            char *name;
            struct Node *type_annotation;
            struct Node *init;
        } var_decl;
        struct {
            struct Node **statements;
            int capacity, count;
        } block;
        struct {
            struct Node *cond;
            struct Node *then_branch;
            struct Node *else_branch;
        } if_stmt;
        struct {
            struct Node *cond;
            struct Node *body;
        } while_stmt;
        struct {
            struct Node *value;
        } ret;
        struct {
            struct Node *expr;
        } expr_stmt;
        struct {
            struct Node **declarations;
            int capacity, count;
        } program;
    }as;
}Node;

typedef struct{
    Node **data;
    size_t size;
    size_t capacity;
}ASTList;


//Node *create_node(Token token, Node *lhs, Node *rhs);
void push_AST(ASTList *AST_list, Node* AST);
void free_AST_list(ASTList *AST_list);
void free_AST(Node *node);
void print_AST(Node *root);

Node *create_int_lit(Token token);
Node *create_float_lit(Token token);
Node *create_ident(Token token);
Node *create_bin_op(Token token, Node *l, Node *r);
Node *create_un_op(Token token, Node* operand);
Node *create_assign(Token token, Node *target, Node *value);
Node *create_type_ref(Token token);
Node *create_var_decl(Token token, Node *init, char *name, Node *type_annotation);
Node *create_block(Token token);
void push_block(Token token);
Node *create_if_stmt(Token token, Node *cond, Node *then_branch, Node *else_branch);
Node *create_while_stmt(Token token, Node *cond, Node *body);
Node *create_ret(Token token, Node *value);
Node *create_expr(Token token, Node *expr);
Node *create_program(Token token);
void push_program(Token token);

#endif