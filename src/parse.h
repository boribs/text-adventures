#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TOKEN_STR_INITIAL_LEN 40
#define TOKEN_STR_LEN_INCREMENT 100

enum ParseState {
    P_STATE_OK,                                   // 0
    P_STATE_UNREACHABLE,                          // 1
    P_STATE_INVALID_CHARACTER_OPENING_ID_DEL,     // 2
    P_STATE_INVALID_CHARACTER_CLOSING_ID_DEL,     // 3
    P_STATE_INVALID_CHARACTER_OPENING_OPTION_DEL, // 4
    P_STATE_INVALID_CHARACTER_CLOSING_OPTION_DEL, // 5
    P_STATE_INVALID_CHAR_IN_ID,                   // 6
    P_STATE_INVALID_LAST_TOKEN,                   // 7
    P_STATE_MISSING_ADVENTURE_DATA,               // 8
    P_STATE_MISSING_AUTHOR,                       // 9
    P_STATE_MISSING_VERSION,                      // 10
    P_STATE_INVALID_SYNTAX_EXPECTED_TEXT,         // 11
    P_STATE_INVALID_SYNTAX_EXPECTED_ID,           // 12
    P_STATE_TOO_MANY_OPTIONS_IN_SECTION,          // 13
    P_STATE_NO_SECTIONS_IN_ADVENTURE,             // 14
};

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
    t->tstr = NULL;
    t->tstr_max_len = 0;
}

static void tok_add_token(struct TokenList *tl, struct Token *t) {
    tl->count++;
    tl->list = realloc(tl->list, sizeof(struct Token) * tl->count);

    tl->list[tl->count - 1] = *t;
    tok_clear(t);
}

static struct Token tok_pop_last_token(struct TokenList *tl) {
    if (tl->count == 0) return (struct Token){.ttype=TOK_EMPTY};

    (tl->count)--;
    struct Token last = tl->list[tl->count];
    tl->list = realloc(tl->list, sizeof(struct Token) * tl->count);

    return last;
}

enum ParseState parse(FILE *file, struct Adventure *a);

#endif // TEXT_ADVENTURES_PARSE
