#include "ast.h"
#include "token.h"

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    setlocale(LC_ALL, "en_US.utf8");

    Tokenizer *t = init_tokenizer(stdin);

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
}