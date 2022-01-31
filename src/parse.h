// parser stuff

#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utf8.h"

#define MIN_SECTION_COUNT 2

enum ParseState {
    P_STATE_OK,                                   // 0
    P_STATE_UNREACHABLE,                          // 1
    P_STATE_INVALID_CHARACTER_OPENING_ID_DEL,     // 2
    P_STATE_INVALID_CHARACTER_CLOSING_ID_DEL,     // 3
    P_STATE_INVALID_CHARACTER_OPENING_OPTION_DEL, // 4
    P_STATE_INVALID_CHARACTER_CLOSING_OPTION_DEL, // 5
    P_STATE_INVALID_CHAR_IN_ID,                   // 6
    P_STATE_MISSING_ID_NUMBER,                    // 7
    P_STATE_INVALID_LAST_TOKEN,                   // 8
    P_STATE_MISSING_ADVENTURE_DATA,               // 9
    P_STATE_MISSING_AUTHOR,                       // 10
    P_STATE_MISSING_VERSION,                      // 11
    P_STATE_INVALID_SYNTAX_EXPECTED_TEXT,         // 12
    P_STATE_INVALID_SYNTAX_EXPECTED_ID,           // 13
    P_STATE_TOO_MANY_OPTIONS_IN_SECTION,          // 14
    P_STATE_NO_SECTIONS_IN_ADVENTURE,             // 15
    P_STATE_VERY_FEW_SECTIONS_IN_ADVENTURE,       // 16
    P_STATE_REPEATED_SECTION_ID,                  // 17
    P_STATE_SELF_POINTING_SECTION,                // 18
    P_STATE_UNREACHABLE_SECTION,                  // 19
    P_STATE_NONEXISTENT_SECTION,                  // 20
};

enum TokenType {
    TOK_ID,
    TOK_TEXT,
    TOK_OPENING_OPTIONS_DELIMITER,
    TOK_CLOSING_OPTIONS_DELIMITER,
    TOK_EMPTY,
};

struct Token {
    enum TokenType type;
    char *str;
    size_t col;
    size_t row;
};

struct TokenList {
    struct Token *list;
    size_t count;
};

struct TokenError {
    enum ParseState state;
    size_t col;
    size_t row;
};

static struct TokenError te(enum ParseState state, size_t col, size_t row) {
    return (struct TokenError) {
        .state = state,
        .col = col,
        .row = row,
    };
}

static struct TokenError te_ok() {
    return (struct TokenError) { .state = P_STATE_OK };
}

static struct TokenError te_un() { // unreachable
    return (struct TokenError) { .state = P_STATE_UNREACHABLE };
}

#endif // TEXT_ADVENTURES_PARSE
