#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "utf8.h"
#include "parse.h"

enum TokenType {
    TOK_NON,
    TOK_STR,
    TOK_NUM,
    TOK_DC,
    // TOK_OBJ,
    // TOK_LIST,
};

utf8char get_char(FILE *stream) {
    utf8_int8_t pool[10] = {0};
    size_t i = 1;

    pool[0] = fgetc(stream);

    while (!feof(stream) && utf8nvalid(pool, i) != 0) {
        pool[i++] = fgetc(stream);
    }
    char *out = utf8ndup(pool, i);

    if (!out) {
        printf("Error extracting char\n");
        exit(1);
    }

    p_col++;
    if (utf8cmp(out, "\n") == 0) {
        p_row++;
        p_col = 0;
    }

    return (utf8char){ .chr = out, .len = i };
}

/*
 * Similar to strcat, but with String and utf8char
 * This also reallocates memory
 */
static void charcat(String *dst, utf8char *s) {
    dst->len += s->len;
    char *p = realloc(dst->chars, dst->len);

    if (p == NULL) {
        printf("Fatal error: can't realloc memory.");
        exit(1);
    }
    dst->chars = p;
    utf8cat(dst->chars, s->chr);
}

static void new_string(String *s) {
    s->len = 1;
    char *p = calloc(1, sizeof(char));

    if (p == NULL) {
        printf("Fatal error: can't calloc memory.");
        exit(1);
    }
    s->chars = p;
}

/*
 * Takes a stream of characters and interprets it  as JSON.
 * Sets parse_error flag on error.
 */
Object json_parse(FILE *stream) {
    p_col = 0;
    p_row = 0;
    utf8char c;

    while (!feof(stream)) {
        c = get_char(stream);

        if (*c.chr < 0) break;

        if (utf8cmp(c.chr, "{") == 0) {
            // create_object sets error flag,
            // no need to check for error

            return create_object(stream);

        } else if (!isutf8whitespace(c.chr)) {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            return (Object){};
        }
    }

    parse_state = PS_ERROR;
    parse_error = PE_EMPTY_FILE; // nothing to parse
    return (Object){};
}

/*
 * Creates an object from a stream of characters.
 * Object parsing ends until matching } is found.
 *
 * Sets parse_error flag on error.
 * TODO: Check for mismatched brackets
 */
Object create_object(FILE *stream) {
    utf8char c;
    Object out = (Object){
        .relation_count = 0,
        .relations = malloc(sizeof(Relation *))
    };

    while (!feof(stream)) {
        c = get_char(stream);

        if (utf8cmp(c.chr, "\"") == 0) {
            Relation rel = create_relation(stream);

            if (parse_state != PS_OK) {
                return out;
            }

            out.relation_count++;
            Relation *r = realloc(out.relations, out.relation_count * sizeof(Relation *));
            assert(r != NULL && "Error allocating memory for relation.");
            r[out.relation_count - 1] = rel;
            out.relations = r;

        } else if (utf8cmp(c.chr, "}") == 0) {
            break;

        } else if (!isutf8whitespace(c.chr)) {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            return out;
        }
    }

    // TODO: check for empty object?
    // TODO: check for incomplete object

    parse_state = PS_OK;
    return out;
}

Relation create_relation(FILE *stream) {
    Relation r = (Relation){.value_type = -1};

    String token = (String){};
    new_string(&token);
    enum TokenType token_type = TOK_STR;
    enum TokenType last_token = TOK_NON;

    bool escaped = false;

    while (!feof(stream)) {
        utf8char uc = get_char(stream);
        char *c = uc.chr;

        if (utf8cmp(c, "\\") == 0) {
            if (token_type != TOK_STR) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            }

            if (escaped) {
                charcat(&token, &uc);
                escaped = false;
            } else {
                escaped = true;
            }

        } else if (utf8cmp(c, "\"") == 0) {
            if (token_type == TOK_STR) {
                if (escaped) {
                    charcat(&token, &uc);
                    escaped = false;
                } else {
                    token_type = TOK_NON;
                    last_token = TOK_STR;
                }
            } else if (last_token != TOK_DC) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            } else {
                last_token = token_type;
                token_type = TOK_STR;
            }

        } else if (utf8cmp(c, ":") == 0) {
            if (token_type == TOK_STR) {
                charcat(&token, &uc);
            } else if (token_type == TOK_NON) {
                if (last_token != TOK_STR || r.key.chars != NULL) {
                    parse_state = PS_ERROR;
                    parse_error = PE_INVALID_CHAR;
                    return r;
                } else {
                    r.key = token;
                    token = (String){};
                    new_string(&token);
                    last_token = TOK_DC;
                }
            }

        } else if (utf8cmp(c, "{") == 0) {
            assert(0 && "nested objects not implemented.");

        } else if (utf8cmp(c, "[") == 0) {
            assert(0 && "object lists not implemented.");

        } else if (utf8cmp(c, "}") == 0) {
            fseek(stream, -1, SEEK_CUR);
            break;

        } else {
            if (token_type != TOK_STR) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            } else {
                charcat(&token, &uc);
            }
        }
    }

    if (*token.chars != 0 && r.key.chars != NULL) {
        r.value_type = VALUE_STR;
        r.value.str = token;
    }

    if (r.value_type == -1) {
        parse_state = PS_ERROR;
        parse_error = PE_MISSING_VALUE;

    } else {
        parse_state = PS_OK;
    }

    return r;
}
