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
    struct TokenList tokens = (struct TokenList){.count=0, .list=NULL};
    struct Token t = {.tstr=NULL};
    tok_clear(&t);

    while (!feof(file)) {
        char c = getc(file);

        if (c == '[') {
            if (t.ttype == TOK_TEXT) {
                tok_add_token(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) {
                t = (struct Token) { .ttype=TOK_OPENING_OPTIONS_DELIMITER };
                tok_add_token(&tokens, &t);
            } else { return false; } // invalid syntax

        } else if (c == ']') {
            if (t.ttype == TOK_TEXT) {
                tok_add_token(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) {
                t = (struct Token) { .ttype=TOK_CLOSING_OPTIONS_DELIMITER };
                tok_add_token(&tokens, &t);
            } else { return false; } // invalid syntax

        } else if (c == '<') {
            if (t.ttype == TOK_TEXT) {
                tok_add_token(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_ID; }
            else { return false; } // invalid char/syntax

        } else if (isdigit(c)) {
            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_TEXT; }
            if (t.ttype == TOK_TEXT || t.ttype == TOK_ID) { tok_addch(c, &t); }
            else { return false; } // unreachable

        } else if (c == '>') {
            if (t.ttype == TOK_TEXT) {
                tok_add_token(&tokens, &t);
            }

            if (t.ttype == TOK_ID) {
                tok_add_token(&tokens, &t);
            } else { return false; } // invalid syntax

        } else if (is_valid_text_token_char(c)) {
            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_TEXT; }
            if (t.ttype == TOK_TEXT) { tok_addch(c, &t); }
            else { return false; } // invalid char
        }

        else if (is_whitespace(c)) {
            if (t.ttype == TOK_TEXT) { tok_addch(c, &t); }
        }
    }

    if (t.ttype != TOK_EMPTY) {
        tok_add_token(&tokens, &t);
    }


    return true;
}
