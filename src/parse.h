#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdbool.h>


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
bool parse(FILE *file, struct Adventure *a);

#endif // TEXT_ADVENTURES_PARSE
