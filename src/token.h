#include "error.h"

#include <stdbool.h>
#include <stdio.h>
#include <wchar.h>

#ifndef TOKEN_H
#define TOKEN_H

enum TokenType {
    tok_eof = -1, // EOF token

    tok_f      = -2, // f token (function def)
    tok_extern = -3, // extern token for specifying C functions
    tok_var    = -4, // var token
    
    tok_ident  = -5, // identifier token
    tok_int    = -6, // int token
    tok_double = -7, // double token

    tok_slash       = -8,  // forward slash / token
    tok_open_paren  = -9,  // open parenthesis ( token
    tok_close_paren = -10, // close parenthesis ) token
    tok_plus        = -11, // plus sign + token
    tok_dash        = -12, // dash sign - token
    tok_star        = -13, // star sign * token
    tok_left_arrow  = -14, // left arrow < token
    tok_right_arrow = -15, // right arrow > token
};

typedef struct {
    enum TokenType type;
    wint_t *token;
} Token;

typedef struct {
    size_t capacity;
    size_t w_position;
    size_t r_position;
    wint_t *buffer;
} Buffer;

typedef struct {
    Buffer *buffer;
    Token *token;
    Error *error;
} Tokenizer;

Tokenizer *init_tokenizer(FILE *file);
Error *next_token(Tokenizer *t);
void deinit_tokenizer(Tokenizer *t);

#endif
