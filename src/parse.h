#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdbool.h>

#define TOKEN_STR_INITIAL_LEN 40
#define TOKEN_STR_LEN_INCREMENT 100

enum TokenType {
    TOK_ID,
    TOK_TEXT,
    TOK_OPENING_OPTIONS_DELIMITER,
    TOK_CLOSING_OPTIONS_DELIMITER,
    TOK_EMPTY,
};

struct Token {
    enum TokenType ttype;
    char *tstr;
    size_t tstr_max_len;
};

static void tok_addch(char c, struct Token *t) {
    if (t->tstr == NULL) {
        t->tstr = (char *)calloc(TOKEN_STR_INITIAL_LEN, sizeof(char));
        t->tstr_max_len = TOKEN_STR_INITIAL_LEN;
    }
    size_t len = strlen(t->tstr);
    if (len + 1 == t->tstr_max_len) {
        t->tstr_max_len += TOKEN_STR_LEN_INCREMENT;
        t->tstr = (char *)realloc(t->tstr, sizeof(char) * t->tstr_max_len);
    }

    t->tstr[len] = c;
    t->tstr[len + 1] = 0;
}

static void tok_clear(struct Token *t) {
    t->ttype = TOK_EMPTY;
    if (t->tstr != NULL) {
        free(t->tstr);
        t->tstr = NULL;
    }
    t->tstr_max_len = 0;
}

static bool add_token(struct Token *tokens, struct Token *t, size_t *count) {
    if (*count == MAX_TOKENS) return false;

    tokens[*count] = *t;
    (*count)++;
    tok_clear(t);
    return true;
}

bool parse(FILE *file, struct Adventure *a);

#endif // TEXT_ADVENTURES_PARSE
