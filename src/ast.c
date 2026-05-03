#include <stdlib.h>
#include "ast.h"
#include "parser.h"

static Node *make_node(NodeType type, int line, int col);
static void free_AST(Node *node);
static bool is_prefix(TokenType op);

Node *create_int_lit(Token token){
    Node *node = make_node(NODE_INT_LIT, token.line, token.col);
    node->as.int_lit.value = token.value.i;
    return node;
}

Node *create_float_lit(Token token){
    Node *node = make_node(NODE_FLOAT_LIT, token.line, token.col);
    node->as.float_lit.value = token.value.f;
    return node;
}

Node *create_ident(Token token){
    Node *node = make_node(NODE_IDENTIFIER, token.line, token.col);
    node->as.ident.name = token.value.s;
    return node;
}

Node *create_bin_op(Token token, Node *l, Node *r){
    Node *node = make_node(NODE_BINARY_OP, token.line, token.col);
    node->as.bin_op.op = token.type;
    node->as.bin_op.l = l;
    node->as.bin_op.r = r;
    return node; 
}

Node *create_un_op(Token token, Node* operand){
    Node *node = make_node(NODE_UNARY_OP, token.line, token.col);
    node->as.un_op.op = token.type;
    node->as.un_op.operand = operand;
    return node; 
}

Node *create_assign(Token token, Node *target, Node *value){
    Node *node = make_node(NODE_ASSIGN, token.line, token.col);
    node->as.assign.op = token.type;
    node->as.assign.target = target;
    node->as.assign.value = value;
    return node; 
}

Node *create_type_ref(Token token){
    Node *node = make_node(NODE_TYPE_REF, token.line, token.col);
    node->as.type_ref.name = token.value.s; 
    return node;
}

Node *create_var_decl(Token token, Node *init, char *name, Node *type_annotation){
    Node *node = make_node(NODE_VAR_DECL, token.line, token.col);
    node->as.var_decl.init = init;
    node->as.var_decl.name = name;
    node->as.var_decl.type_annotation = type_annotation;
    return node; 
}

Node *create_block(Token token){
    Node *node = make_node(NODE_BLOCK, token.line, token.col);
    node->as.block.statements = NULL;
    node->as.block.count = 0;
    node->as.block.capacity = 0;
    return node; 
}

void push_block(Token token){

}

Node *create_if_stmt(Token token, Node *cond, Node *then_branch, Node *else_branch){
    Node *node = make_node(NODE_IF,token.line,token.col);
    node->as.if_stmt.cond = cond;
    node->as.if_stmt.then_branch = then_branch;
    node->as.if_stmt.else_branch = else_branch;
    return node; 
}

Node *create_while_stmt(Token token, Node *cond, Node *body){
    Node *node = make_node(NODE_WHILE,token.line,token.col);
    node->as.while_stmt.cond = cond;
    node->as.while_stmt.body = body;
    return node; 
}

Node *create_ret(Token token, Node *value){
    Node *node = make_node(NODE_RETURN,token.line,token.col);
    node->as.ret.value = value;
    return node;  
}

Node *create_expr(Token token, Node *expr){
    Node *node = make_node(NODE_EXPR_STMT,token.line,token.col);
    node->as.expr_stmt.expr = expr;
    return node; 
}

Node *create_program(Token token){
    Node *node = make_node(NODE_PROGRAM,token.line,token.col);
    node->as.program.declarations = NULL;
    node->as.program.count = 0;
    node->as.program.capacity = 0;
    return node; 
}

void push_program(Token token){

}

void push_AST(ASTList *AST_list, Node* AST){
    if(AST_list->capacity == AST_list->size){
        AST_list->capacity = AST_list->capacity == 0 ? 8 : 2*AST_list->capacity;
        AST_list->data = realloc(AST_list->data,AST_list->capacity*sizeof(Node *));
    }
    AST_list->data[AST_list->size++] = AST;
}

void free_AST_list(ASTList *AST_list){
    for (size_t i = 0; i < AST_list->size; i++){
        free_AST(AST_list->data[i]);
    }
    free(AST_list->data);
}

void free_AST(Node *node){
    if(node == NULL) return;
    switch (node->type){
        case NODE_INT_LIT:
        case NODE_FLOAT_LIT:
            break;
        case NODE_IDENTIFIER:
            free(node->as.ident.name);
            break;
        case NODE_BINARY_OP:
            free_AST(node->as.bin_op.l);
            free_AST(node->as.bin_op.r);
            break;
        case NODE_UNARY_OP:
            free_AST(node->as.un_op.operand);
            break;
        case NODE_ASSIGN:
            free_AST(node->as.assign.target);
            free_AST(node->as.assign.value);
            break;
        case NODE_TYPE_REF:
            free(node->as.type_ref.name);
            break;
        case NODE_VAR_DECL:
            free(node->as.var_decl.name);
            free_AST(node->as.var_decl.type_annotation);
            free_AST(node->as.var_decl.init);
            break;
        case NODE_BLOCK:
            for (size_t i = 0; i < node->as.block.count; i++){
                free_AST(node->as.block.statements[i]);
            }
            break;
        case NODE_IF:
            free_AST(node->as.if_stmt.cond);
            free_AST(node->as.if_stmt.then_branch);
            free_AST(node->as.if_stmt.else_branch);
            break;
        case NODE_WHILE:
            free_AST(node->as.while_stmt.cond);
            free_AST(node->as.while_stmt.body);
            break;
        case NODE_RETURN:
            free_AST(node->as.ret.value);
            break;
        case NODE_EXPR_STMT:
            free_AST(node->as.expr_stmt.expr);
            break;
        case NODE_PROGRAM:
            for(size_t i = 0; i < node->as.program.count; i++){
                free_AST(node->as.program.declarations[i]);
            }
            break;
        default:
            printf("Ill-defined node");
            break;
    }
    free(node);
}

void print_AST(Node *root){
    print_AST_indented(root, 0);
}

static void print_AST_indented(Node *node, int depth){
    for(size_t i = 0; i < depth; i++) printf("  ");
    if(node == NULL){
        printf("NULL");
        return; 
    }
    switch (node->type){
    case NODE_INT_LIT:    
        printf("%d\n",node->as.int_lit.value);
        break;
    case NODE_FLOAT_LIT:  
        printf("%f\n",node->as.float_lit.value);
        break;
    case NODE_IDENTIFIER: 
        char *name = node->as.ident.name;
        printf("%s\n", name ? name : "Null");
        break; 
    case NODE_BINARY_OP:
        printf("Binary operator: %s\n", token_type_names[node->as.bin_op.op]);
        print_AST_indented(node->as.bin_op.l, depth+1);
        print_AST_indented(node->as.bin_op.r, depth+1);
        break;
    case NODE_UNARY_OP:
        printf("Unary operator: %s", token_type_names[node->as.un_op.op]);
        if (is_prefix(node->as.un_op.op)){
            printf(" (Prefix)\n");
        } else printf(" (Suffix)\n");
        print_AST_indented(node->as.un_op.operand, depth+1);
        break;
    case NODE_ASSIGN:
        printf("Assign: %s\n", token_type_names[node->as.assign.op]);
        print_AST_indented(node->as.assign.target, depth+1);
        print_AST_indented(node->as.assign.value, depth+1);
        break;
    case NODE_VAR_DECL:
        char *name = node->as.var_decl.name;
        Node *type = node->as.var_decl.type_annotation;
        printf("Let:\n");
        if (!node->as.var_decl.type_annotation){
            print_AST_indented(node->as.var_decl.type_annotation, depth+1);
        } else {
            for (size_t i = 0; i < depth + 1; i++) printf("  ");
            printf("Inferred type\n");
        }
        for (size_t i = 0; i < depth + 1; i++) printf("  ");
        printf("%s\n", name ? name : "Null");
        print_AST_indented(node->as.var_decl.init, depth+1);
        break;
    case NODE_BLOCK:
        printf("Block:\n");
        for (size_t i = 0; i < node->as.block.count; i++){
            print_AST_indented(node->as.block.statements[i],depth+1);
        }
        break;
    case NODE_IF:
        printf("If:\n");
        print_AST_indented(node->as.if_stmt.cond, depth+1);
        printf("Then:\n");
        print_AST_indented(node->as.if_stmt.then_branch, depth+1);
        printf("Else:\n");
        print_AST_indented(node->as.if_stmt.else_branch, depth+1);
        break;
    case NODE_WHILE:
        printf("While:\n");
        print_AST_indented(node->as.while_stmt.cond, depth+1);
        printf("Do:\n");
        print_AST_indented(node->as.while_stmt.body, depth+1);
        break;
    case NODE_RETURN:
        printf("Return:\n");
        print_AST_indented(node->as.ret.value, depth+1);
        break;
    case NODE_EXPR_STMT:
        printf("Expression statement:\n");
        print_AST_indented(node->as.expr_stmt.expr, depth+1);
        break;
    case NODE_PROGRAM:
        printf("Program:\n");
        for (size_t i = 0; i < node->as.program.count; i++){
            print_AST_indented(node->as.program.declarations[i], depth+1);
        }
        break;
    default:
        printf("Ill-defined node"); break;
    }
}

static Node *make_node(NodeType type, int line, int col){
    Node *node = safe_malloc(sizeof(Node));
    node->type = type;
    node->result_type = NULL;
    node->line = line;
    node->col = col;
    return node;
}

static bool is_prefix(TokenType op){
    if (op == TOKEN_PLUS || op == TOKEN_MINUS){
        return true;
    }
    else {
        // NOT IMPLEMENTED
        return false;
    }
}
