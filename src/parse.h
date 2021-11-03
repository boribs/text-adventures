#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

struct TokenList {
    struct Token *list;
    size_t count;
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
        t->tstr = NULL;
    }
    t->tstr_max_len = 0;
}

static void tok_grow_list(struct TokenList *tl) {
    tl->count++;
    tl->list = realloc(tl->list, sizeof(struct Token) * tl->count);
}

static void tok_add_token(struct TokenList *tl, struct Token *t) {
    tok_grow_list(tl);
    tl->list[tl->count - 1] = *t;
    tok_clear(t);
}

bool parse(FILE *file, struct Adventure *a);

#endif // TEXT_ADVENTURES_PARSE
