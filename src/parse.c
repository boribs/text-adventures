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

static char * trim_r(char *s) {
    for (size_t i = strlen(s) - 1; i >= 0; --i) {
        if (!is_whitespace(s[i])) {
            s[i + 1] = 0;
            return realloc(s, sizeof(char) * (i + 2));
        }
    }

    return s;
}

static void add_token_to_list(struct TokenList *tl, struct Token *t) {
    if (t->tstr != NULL) {
        t->tstr = trim_r(t->tstr);
    }
    tok_add_token(tl, t);
}

static bool construct_option(struct TokenList *tl, struct Token *t) {
    struct Opt option;
    struct Token token = *t;

    if (token.ttype != TOK_TEXT) return false;
    option.text = token.tstr;

    token = tok_pop_last_token(tl);
    if (token.ttype != TOK_ID) return false;
    option.sec_id = strtol(token.tstr, NULL, 10);
    free(token.tstr);

    return true;
}

static bool construct_options(struct TokenList *tl) {
    struct Token t = tok_pop_last_token(tl);

    while (t.ttype != TOK_OPENING_OPTIONS_DELIMITER) {
        construct_option(tl, &t);
        t = tok_pop_last_token(tl);
    }
}

static struct Sec construct_section(struct TokenList *tl) {
    struct Sec s;

    construct_options(tl);
}

bool parse(FILE *file, struct Adventure *a) {
    struct TokenList tokens = (struct TokenList){.count=0, .list=NULL};
    struct Token t = {.tstr=NULL};
    tok_clear(&t);

    while (!feof(file)) {
        char c = getc(file);

        if (c == '[') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) {
                t = (struct Token) { .ttype=TOK_OPENING_OPTIONS_DELIMITER };
                add_token_to_list(&tokens, &t);
            } else { return false; } // invalid syntax

        } else if (c == ']') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            } else if (t.ttype != TOK_EMPTY) {
                return false; // invalid syntax
            }

            // if (t.ttype == TOK_EMPTY) {
            //     t = (struct Token) { .ttype=TOK_CLOSING_OPTIONS_DELIMITER };
            //     add_token_to_list(&tokens, &t);
            // } else { return false; } // invalid syntax

            construct_section(&tokens);

        } else if (c == '<') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_ID; }
            else { return false; } // invalid char/syntax

        } else if (isdigit(c)) {
            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_TEXT; }
            if (t.ttype == TOK_TEXT || t.ttype == TOK_ID) { tok_addch(c, &t); }
            else { return false; } // unreachable

        } else if (c == '>') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_ID) {
                add_token_to_list(&tokens, &t);
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
        return false; // last token should be "]", which
                      // is handled on read
    }


    return true;
}
