#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "common.h"
#include "parse.h"
#include "utf8.h"

static bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

static char *trim_r(char *s) {
    for (size_t i = utf8size_lazy(s) - 1; i >= 0; --i) {
        if (!is_whitespace(s[i])) {
            s[i + 1] = 0;
            char *n = utf8ndup(s, i + 1);
            free(s);

            return n;
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

static struct TokenError construct_option(struct TokenList *tl, struct Token *t, struct Opt *out) {
    struct Token token = *t;

    if (token.ttype != TOK_TEXT) {
        return te(P_STATE_INVALID_SYNTAX_EXPECTED_TEXT, token.col, token.row); // invalid syntax - expected text
    }
    out->text = token.tstr;

    token = tok_pop_last_token(tl);
    if (token.ttype != TOK_ID) {
        return te(P_STATE_INVALID_SYNTAX_EXPECTED_ID, token.col, token.row); // invalid syntax - expected id
    }
    out->sec_id = strtol(token.tstr, NULL, 10);
    free(token.tstr);

    return te_ok();
}

static struct TokenError construct_options(struct TokenList *tl, struct Sec *out) {
    struct Token t = tok_pop_last_token(tl);
    struct Opt *options = malloc(sizeof(struct Opt) * MAX_OPTION_COUNT);
    size_t opt_count = 0;

    while (t.ttype != TOK_OPENING_OPTIONS_DELIMITER) {
        if (opt_count == MAX_OPTION_COUNT) {
            return te(P_STATE_TOO_MANY_OPTIONS_IN_SECTION, t.col, t.row); // too many options!
        }

        struct TokenError terr = construct_option(tl, &t, options + opt_count);
        if (terr.state != P_STATE_OK) return terr;
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
        out->options = realloc(options, sizeof(struct Opt) * opt_count);
    }
    out->opt_count = opt_count;

    return te_ok();
}

static struct TokenError construct_section(struct TokenList *tl, struct Sec *s) {
    struct TokenError terr = construct_options(tl, s);
    if (terr.state != P_STATE_OK) return terr;

    struct Token t = tok_pop_last_token(tl);
    if (t.ttype != TOK_TEXT) {
        return te(P_STATE_INVALID_SYNTAX_EXPECTED_TEXT, t.col, t.row); // invalid syntax - expected text
    }
    s->text = t.tstr;

    t = tok_pop_last_token(tl);
    if (t.ttype != TOK_ID) {
        return te(P_STATE_INVALID_SYNTAX_EXPECTED_ID, t.col, t.row); // invalid syntax - expected id
    }
    s->id = strtol(t.tstr, NULL, 10);
    free(t.tstr);

    return te_ok();
}

static char *read_char(FILE *file, size_t *byte_count) {
    *byte_count = 0;
    char tmp[7] = { 0 }; // or however many bits a char can have
    char *tptr = &tmp[0];

    do {
        fread(tptr++, 1, 1, file);
        (*byte_count)++;
    } while(utf8valid(tmp) != 0);

    return utf8dup(tmp);
}

struct TokenError parse(FILE *file, struct Adventure *a) {
    struct Sec *sections = NULL;
    size_t section_count = 0;

    struct TokenList tokens = (struct TokenList){.count=0, .list=NULL};
    struct Token t = {.tstr=NULL};
    size_t col = 0, row = 1;
    tok_clear(&t);

    struct TokenError terr;

    while (!feof(file)) {
        size_t byte_count = 0;
        char *char_str = read_char(file, &byte_count); // does this need to be free'd?

        int c = -1;
        if (byte_count == 1) {
            c = *char_str;
        }

        if (c == '[') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) {
                t = (struct Token) { .ttype=TOK_OPENING_OPTIONS_DELIMITER, .col=col, .row=row };
                add_token_to_list(&tokens, &t);
            } else {
                return te(P_STATE_INVALID_CHARACTER_OPENING_OPTION_DEL, col, row); // invalid char - found [ in ID
            }
            col++;

        } else if (c == ']') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            } else if (t.ttype != TOK_EMPTY) {
                return te(P_STATE_INVALID_CHARACTER_CLOSING_OPTION_DEL, col, row); // invalid char - found ] in ID
            }

            col++;
            section_count++;
            sections = realloc(sections, sizeof(struct Sec) * section_count);
            terr = construct_section(&tokens, sections + section_count - 1);
            if (terr.state != P_STATE_OK) return terr;

        } else if (c == '<') {
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_ID; t.col = col; t.row = row; }
            else { return te(P_STATE_INVALID_CHARACTER_OPENING_ID_DEL, col, row); } // invalid char - found < inside ID
            col++;

        } else if (isdigit(c)) {
            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_TEXT; t.col = col; t.row = row; }
            if (t.ttype == TOK_TEXT || t.ttype == TOK_ID) { tok_addch(char_str, byte_count, &t); }
            else { return te_un(); } // unreachable
            col++;

        } else if (c == '>') {
            col++;
            if (t.ttype == TOK_TEXT) {
                add_token_to_list(&tokens, &t);
            }

            if (t.ttype == TOK_ID) {
                if (t.tstr == NULL) { return te(P_STATE_MISSING_ID_NUMBER, t.col, t.row); }

                add_token_to_list(&tokens, &t);
            } else { return te(P_STATE_INVALID_CHARACTER_CLOSING_ID_DEL, col, row); } // invalid syntax - found > outside of ID

        } else if (is_whitespace(c)) {
            col++;
            if (t.ttype == TOK_TEXT) { tok_addch(char_str, byte_count, &t); }
            if (c == '\n') { row++; col = 1; }

        } else {
            if (c == 0) continue;

            if (t.ttype == TOK_EMPTY) { t.ttype = TOK_TEXT; t.col = col; t.row = row; }
            if (t.ttype == TOK_TEXT) { tok_addch(char_str, byte_count, &t); }
            else if (t.ttype == TOK_ID) { return te(P_STATE_INVALID_CHAR_IN_ID, col, row); } // invalid char - non-numeric value inside ID
            col++;
        }
    }

    if (t.ttype != TOK_EMPTY) {
        return te(P_STATE_INVALID_LAST_TOKEN, 0, 0); // last token should be "]", which
                                                     // is handled on read
    }

    if (section_count < MIN_SECTION_COUNT) {
        return te(P_STATE_VERY_FEW_SECTIONS_IN_ADVENTURE, 0, 0);
    }

    t = tok_pop_last_token(&tokens);
    if (t.ttype != TOK_TEXT) return te(P_STATE_MISSING_ADVENTURE_DATA, 0, 0); // invalid first token - expected text

    char tmp[utf8size(t.tstr)];
    utf8cpy(tmp, t.tstr);
    free(t.tstr);

    char *tok = strtok(tmp, "\n");
    if (tok == NULL) return te_un(); // invalid first line - expected title
    a->title = trim_r(utf8dup(tok));

    tok = strtok(NULL, "\n");
    if (tok == NULL) return te(P_STATE_MISSING_AUTHOR, t.col, t.row); // invalid second line - expected author
    a->author = trim_r(utf8dup(tok));

    tok = strtok(NULL, "\n");
    if (tok == NULL) return te(P_STATE_MISSING_VERSION, t.col, t.row); // invalid third line - expected version
    a->version = trim_r(utf8dup(tok));

    // ignore the rest of the first token

    free(tokens.list);

    // check for repeated sections
    for (size_t i = 0; i < section_count; ++i) {
        for (size_t j = 1; j < section_count; ++j) {
            if (i == j) continue;

            if (sections[i].id == sections[j].id) {
                return te(P_STATE_REPEATED_SECTION_ID, sections[j].id, 0);
            }
        }
    }

    // check for self-pointing section
    for (size_t i = 0; i < section_count; ++i) {
        for (size_t j = 0; j < sections[i].opt_count; ++j) {
            if (sections[i].options[j].sec_id == sections[i].id) {
                return te(P_STATE_SELF_POINTING_SECTION, sections[j].id, 0);
            }
        }
    }

    // check for unreachable sections
    for (size_t i = 1; i < section_count; ++i) {
        size_t id = sections[i].id;
        bool found = false;

        for (size_t j = 0; j < section_count; ++j) {
            if (i == j) continue;

            for (size_t o = 0; o < sections[j].opt_count; ++o) {
                if (sections[j].options[o].sec_id == id) {
                    found = true;
                    break;
                }
            }

            if (found) break;
        }

        if (!found) {
            return te(P_STATE_UNREACHABLE_SECTION, id, 0);
        }
    }

    // check for nonexistent sections
    for (size_t i = 0; i < section_count; ++i) {
        for (size_t o = 0; o < sections[i].opt_count; ++o) {
            size_t oid = sections[i].options[o].sec_id;
            bool found = false;

            for (size_t j = 0; j < section_count; ++j) {
                if (i == j) continue;

                if (sections[j].id == oid) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return te(P_STATE_NONEXISTENT_SECTION, sections[i].id, oid);
            }
        }
    }

    a->sections = sections;
    a->sec_count = section_count;

    return te_ok();
}
