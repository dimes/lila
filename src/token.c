#include "token.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wctype.h>

const wint_t non_ident[8] = {'/', '+', '-', '*', '(', ')', '<', '>'};
const size_t non_ident_size = sizeof(non_ident) / sizeof(non_ident[0]);

const wint_t *reserved_f = L"f";
const wint_t *reserved_var = L"var";
const wint_t *reserved_extern = L"extern";

Buffer *new_buffer(size_t initial_capacity) {
    Buffer *buffer = malloc(sizeof(Buffer));
    memset(buffer, 0, sizeof(Buffer));

    buffer->capacity = initial_capacity;
    buffer->buffer = malloc(initial_capacity * sizeof(wint_t));
    memset(buffer->buffer, 0, initial_capacity * sizeof(wint_t));

    return buffer;
}

void expand_buffer(Buffer *buffer) {
    size_t new_capacity = buffer->capacity * 2;
    wint_t *new_buffer = malloc(new_capacity * sizeof(wint_t));
    memset(new_buffer, 0, new_capacity * sizeof(wint_t));

    memcpy(new_buffer, buffer->buffer, buffer->capacity * sizeof(wint_t));
    free(buffer->buffer);

    buffer->capacity = new_capacity;
    buffer->buffer = new_buffer;
}

void append_to_buffer(Buffer *buffer, wint_t c) {
    size_t pos = buffer->w_position;
    while (buffer->capacity < pos) {
        expand_buffer(buffer);
    }

    buffer->buffer[pos] = c;
    buffer->w_position += 1;
}

void dealloc_buffer(Buffer *buffer) {
    free(buffer->buffer);
    free(buffer);
}

Tokenizer *init_tokenizer(FILE *file) {
    Tokenizer *t = malloc(sizeof(Tokenizer));
    memset(t, 0, sizeof(Tokenizer));

    t->buffer = new_buffer(1024);

    wint_t c;
    errno = 0;
    Buffer *buffer = new_buffer(1024);
    while ((c = fgetwc(file)) != WEOF) {
        append_to_buffer(t->buffer, c);
    }

    if (ferror(file)) {
        t->error = malloc(sizeof(Error));
        memset(t->error, 0, sizeof(Error));
        if (errno == EILSEQ) {
            t->error->message = "Encoding error";
        } else {
            t->error->message = "I/O error reading file";
        }
    }

    return t;
}

void set_token(Tokenizer *t, enum TokenType token_type, void *token) {
    t->token = malloc(sizeof(Token));
    memset(t->token, 0, sizeof(Token));
    t->token->type = token_type;
    t->token->token = token;
}

void free_token(Tokenizer *t) {
    if (t->token) {
        if (t->token->token) {
            free(t->token->token);
            t->token->token = NULL;
        }

        free(t->token);
        t->token = NULL;
    }
}

wint_t next_char(Tokenizer *t, size_t pos) {
    if (pos < 0 || pos >= t->buffer->w_position) {
        return 0;
    }

    return t->buffer->buffer[pos];
}

bool isdelim(wint_t c) {
    if (c == 0 || isspace(c)) {
        return true;
    }

    size_t i;
    for (i = 0; i < non_ident_size; i++) {
        if (c == non_ident[i]) {
            return true;
        }
    }

    return false;
}

bool isdouble(wint_t *number, size_t length) {
    size_t i;
    for (i = 0; i < length; i++) {
        if (number[i] == '.') {
            return true;
        }
    }
    return false;
}

size_t read_single_line_comment(Tokenizer *t, size_t start) {
    size_t i;
    for (i = start; i < t->buffer->w_position; i++) {
        if (t->buffer->buffer[i] == '\n') {
            break;
        }
    }
    return 1 + i - start;
}

size_t read_multi_line_comment(Tokenizer *t, size_t start) {
    size_t i;
    wint_t last_char = 0;
    for (i = start + 2; i < t->buffer->w_position; i++) {
        if (last_char == '*' && t->buffer->buffer[i] != '/') {
            break;
        }
        last_char = t->buffer->buffer[i];
    }
    return 1 + i - start;
}

size_t read_ident(Tokenizer *t, size_t start) {
    size_t i;
    for (i = start; i < t->buffer->w_position; i++) {
        wint_t next = next_char(t, i+1);
        if (isdelim(next)) {
            break;
        }
    }
    return 1 + i - start;
}

size_t read_number(Tokenizer *t, size_t start) {
    size_t i;
    for (i = start; i < t->buffer->w_position; i++) {
        wint_t next = next_char(t, i+1);
        if (!isdigit(next) && next != '.') {
            break;
        }
    }
    return i - start;
}

Error *next_token(Tokenizer *t) {
    free_token(t);

    if (t->error) {
        return t->error;
    }

    for (; t->buffer->r_position < t->buffer->w_position; t->buffer->r_position++) {
        size_t i = t->buffer->r_position;
        wint_t c = t->buffer->buffer[i];
        fprintf(stderr, "Character is '%c' at position %ld\n", c, t->buffer->r_position);
        if (iswspace(c)) {
            printf("Skipping space '%c'\n", c);
            continue;
        }

        if (c == '/') {
            if (next_char(t, i+1) == '/') {
                fprintf(stderr, "Reading single line comment\n");
                t->buffer->r_position += read_single_line_comment(t, i) - 1;
                continue;
            } else if (next_char(t, i+1) == '*') {
                fprintf(stderr, "Reading multiline comment\n");
                t->buffer->r_position += read_multi_line_comment(t, i) - 1;
                continue;
            }

            set_token(t, tok_slash, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '+') {
            set_token(t, tok_plus, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '-') {
            set_token(t, tok_dash, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '*') {
            set_token(t, tok_star, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '(') {
            set_token(t, tok_open_paren, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == ')') {
            set_token(t, tok_close_paren, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '<') {
            set_token(t, tok_left_arrow, NULL);
            t->buffer->r_position++;
            break;
        }

        if (c == '>') {
            set_token(t, tok_right_arrow, NULL);
            t->buffer->r_position++;
            break;
        }

        if (isdigit(c)) {
            size_t end = read_number(t, i);
            size_t length = end + 1; // +2 for null-terminated
            wint_t *number = malloc(length * sizeof(wint_t));
            memset(number, 0, length * sizeof(wint_t));
            memcpy(number, t->buffer->buffer + (i * sizeof(wint_t)), end * sizeof(wint_t));

            enum TokenType token_type = tok_int;
            if (isdouble(number, length)) {
                token_type = tok_double;
            }
            
            set_token(t, token_type, number);
            t->buffer->r_position += end;
            break;
        }

        if (!isdelim(c)) {
            size_t end = read_ident(t, i);
            size_t length = end + 1; // +1 for null-terminated
            wint_t *ident = malloc(length * sizeof(wint_t));
            memset(ident, 0, length * sizeof(wint_t));
            memcpy(ident, &(t->buffer->buffer[i]), end * sizeof(wint_t));
            fprintf(stderr, "Found ident '%ls' of length %ld\n", ident, end);

            enum TokenType token_type = tok_ident;
            if (wcscmp(reserved_f, ident) == 0) {
                token_type = tok_f;
            } else if (wcscmp(reserved_var, ident) == 0) {
                token_type = tok_var;
            } else if (wcscmp(reserved_extern, ident) == 0) {
                token_type = tok_extern;
            }

            set_token(t, token_type, ident);
            t->buffer->r_position += end;
            break;
        }
    }

    if (t->token == NULL && t->buffer->r_position >= t->buffer->w_position) {
        set_token(t, tok_eof, NULL);
        return NULL;
    }

    return NULL;
}

void deinit_tokenizer(Tokenizer *t) {
    free_token(t);
    dealloc_buffer(t->buffer);
    t->buffer = NULL;

    if (t->error != NULL) {
        free(t->error);
        t->error = NULL;
    }

    free(t);
}
