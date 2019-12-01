#include "ast.h"
#include "token.h"

#include <stdlib.h>

const enum TokenType arithmetic_priorities[6] = { 
    tok_left_arrow, 
    tok_right_arrow, 
    tok_plus,
    tok_dash,
    tok_slash,
    tok_star,
};

ASTNode *parse_file(FILE *file) {
    Tokenizer *t = init_tokenizer(file);

    Token *token = NULL;
    while (token == NULL || token->type != tok_eof) {
        Error *error = next_token(t);
        if (error != NULL) {
            printf("Error: %s", error->message);
            break;
        }

        token = t->token;
        printf("Token %d\n", token->type);
    }

    deinit_tokenizer(t);
    t = NULL;

    return NULL;
}

void free_ast(ASTNode *root) {
    if (root == NULL) {
        return;
    }

    switch(root->type) {
        case node_binary_expr: {
            BinaryExpr *binary_expr = root->expr;
            free_ast(binary_expr->lhs);
            free_ast(binary_expr->rhs);
            break;
        }
        case node_call: {
            CallExpr *call_expr = root->expr;

            int i;
            for (i = 0; i < call_expr->arg_count; i++) {
                free_ast(call_expr->args[i]);
            }

            free(call_expr->callee);
            call_expr->callee = NULL;
            call_expr->arg_count = 0;
            call_expr->args = NULL;
            break;
        }
        case node_function: {
            FunctionExpr *func_expr = root->expr;

            int i;
            for (i = 0; i < func_expr->arg_count; i++) {
                free_ast(func_expr->args[i]);
            }

            free(func_expr->name);
            func_expr->name = NULL;
            func_expr->arg_count = 0;
            func_expr->args = NULL;
            break;
        }
        default: {
            break;
        }
    }

    if (root->expr) {
        free(root->expr);
        root->expr = NULL;
    }

    free(root);
}
