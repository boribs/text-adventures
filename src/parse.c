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

static bool construct_option(struct TokenList *tl, struct Token *t, struct Opt *out) {
    struct Token token = *t;

    if (token.ttype != TOK_TEXT) return false;
    out->text = token.tstr;

    token = tok_pop_last_token(tl);
    if (token.ttype != TOK_ID) return false;
    out->sec_id = strtol(token.tstr, NULL, 10);
    free(token.tstr);

    return true;
}

static bool construct_options(struct TokenList *tl, struct Sec *out) {
    struct Token t = tok_pop_last_token(tl);
    struct Opt *options = malloc(sizeof(struct Opt) * MAX_OPTION_COUNT);
    size_t opt_count = 0;

    while (t.ttype != TOK_OPENING_OPTIONS_DELIMITER) {
        if (opt_count == MAX_OPTION_COUNT) return false; // too many options!

        if (!construct_option(tl, &t, options + opt_count)) return false; // error parsing option
        // printf("<%zu> '%s'\n", options[opt_count].sec_id, options[opt_count].text);
        opt_count++;

        t = tok_pop_last_token(tl);
    }

    // invert options, necessary because they're read backwards
    struct Opt tmp;
    for (size_t i = 0; i < opt_count / 2; ++i) {
        tmp = options[i];
        options[i] = options[opt_count - i - 1];
        options[opt_count - i - 1] = tmp;
    }

    if (opt_count == 0) {
        out->options = NULL;
        free(options);
    } else {
        out->options = options;
    }
    out->opt_count = opt_count;

    return true;
}

static bool construct_section(struct TokenList *tl, struct Sec *s) {
    if (!construct_options(tl, s)) return false;

    struct Token t = tok_pop_last_token(tl);
    if (t.ttype != TOK_TEXT) return false; // invalid syntax - expected text
    s->text = t.tstr;

    t = tok_pop_last_token(tl);
    if (t.ttype != TOK_ID) return false; // invalid syntax - expected id
    s->id = strtol(t.tstr, NULL, 10);
    free(t.tstr);

    return true;
}

static char *copy_to_mem(char *s) {
    char *out = malloc(sizeof(char) * (strlen(s) + 1));
    strcpy(out, s);
    s[strlen(s)] = 0;

    return out;
}

bool parse(FILE *file, struct Adventure *a) {
    struct Sec *sections = NULL;
    size_t section_count = 0;
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

            section_count++;
            sections = realloc(sections, sizeof(struct Sec) * section_count);
            if (!construct_section(&tokens, sections + section_count - 1)) return false; // invalid syntax

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

    t = tok_pop_last_token(&tokens);
    if (t.ttype != TOK_TEXT) return false; // invalid first token - expected text

    char tmp[strlen(t.tstr) + 1];
    tmp[strlen(tmp)] = 0;
    strcpy(tmp, t.tstr);
    free(t.tstr);

    char *tok = strtok(tmp, "\n");
    if (tok == NULL) return false; // invalid first line - expected title - unreachable?
    a->title = trim_r(copy_to_mem(tok));

    tok = strtok(NULL, "\n");
    if (tok == NULL) return false; // invalid second line - expected author
    a->author = trim_r(copy_to_mem(tok));

    tok = strtok(NULL, "\n");
    if (tok == NULL) return false; // invalid third line - expected version
    a->version = trim_r(copy_to_mem(tok));

    // ignore everything in between:)

    a->sections = sections;
    a->sec_count = section_count;
    return true;
}
