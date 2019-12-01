#include <stdio.h>

#ifndef AST_H
#define AST_H

enum ExprType {
    node_int         = -1,
    node_double      = -2,
    node_variable    = -3,
    node_binary_expr = -4,
    node_call        = -5,
    node_function    = -6,
};

typedef struct {
    enum ExprType type;
    void *expr;
} ASTNode;

typedef struct {
    int val;
} IntExpr;

typedef struct {
    double val;
} DoubleExpr;

typedef struct {
    char *name;
} VariableExpr;

typedef struct {
    ASTNode *lhs;
    ASTNode *rhs;
} BinaryExpr;

typedef struct {
    char *callee;
    int arg_count;
    ASTNode **args;
} CallExpr;

typedef struct {
    char *name;
    int arg_count;
    ASTNode **args;
} FunctionExpr;

ASTNode *parse_file(FILE *file);
void free_ast(ASTNode *root);

#endif