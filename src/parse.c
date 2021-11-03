#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "common.h"
#include "parse.h"

static bool is_valid_text_token_char(char c) {
    return ((int)c >= 33 && (int)c <= 126);
}

static bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

bool parse(FILE *file, struct Adventure *a) {
    struct Token tokens[MAX_TOKENS];
    size_t token_count = 0;
    struct Token t = {.tstr=NULL};
    tok_clear(&t);

    while (!feof(file)) {
        char c = getc(file);

        if (c == '[') {


        } else if (c == ']') {


        } else if (c == '<') {


        } else if (isdigit(c)) {


        } else if (c == '>') {


        } else if (is_valid_text_token_char(c)) {


        }

        else if (is_whitespace(c)) {


        }
    }

    if (t.ttype != TOK_EMPTY) {
        add_token(tokens, &t, &token_count);
    }


    return true;
}
