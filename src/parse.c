#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

#include "utf8.h"
#include "parse.h"

enum TokenType {
    TOK_NON,
    TOK_STR,
    TOK_NUM,
    TOK_DC,
    TOK_LIST,
    // TOK_OBJ,
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

    p_prev_col = p_col;
    p_col++;
    if (utf8cmp(out, "\n") == 0) {
        p_row++;
        p_col = 0;
    }

    return (utf8char){ .chr = out, .len = i };
}

/*
 * Seeks the stream back one character and updates
 * p_col and  p_row.
 */
void return_char(FILE *stream, utf8char c) {
    fseek(stream, -c.len, SEEK_CUR);

    if (p_col == 0) {
        p_row--;
    } else {
        p_col = p_prev_col;
    }
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

static void new_list(List *l) {
    l->object_count = 0;
    Object *o = malloc(sizeof(Object));

    if (o == NULL) {
        printf("Fatal error: can't malloc memory.");
        exit(1);
    }
    l->elements = o;
}

/*
 * Parses a series of characters until a non-escaped " is found.
 * This assumes the first " is not part of the character set to parse.
 */
static String create_string(FILE *stream) {
    String out = (String){};
    new_string(&out);
    bool escape = false, complete = false;

    while (!feof(stream)) {
        utf8char c = get_char(stream);

        if (utf8cmp(c.chr, "\"") == 0) {
            if (escape) {
                charcat(&out, &c);
                escape = false;
            } else {
                complete = true;
                break;
            }

        } else if (utf8cmp(c.chr, "\\") == 0) {
            if (escape) {
                charcat(&out, &c);
                escape = false;
            } else {
                escape = true;
            }

        } else if (c.len == 0 && *c.chr < 0) {
            parse_state = PS_ERROR;
            parse_error = PE_MISSING_DOUBLE_QUOTES;
            return out;

        } else {
            charcat(&out, &c);
        }
    }

    if (complete) {
        parse_state = PS_OK;
    } else {
        parse_state = PS_ERROR;
        parse_error = PE_MISSING_DOUBLE_QUOTES;
    }

    return out;
}

/*
 * Parses a series of characters until a non-numeric character is found.
 * Returns parsed number.
 * If an invalid char is found, sets error flag.
 */
static size_t create_number(FILE *stream) {
    size_t num = 0;
    utf8char c;

    while (!feof(stream)) {
        if (num > MAX_NUMERIC_VALUE) {
            parse_state = PS_ERROR;
            parse_error = PE_NUMBER_TOO_BIG;
            break;
        }

        c = get_char(stream);

        if (c.len == 1 && isdigit(*c.chr)) {
            num = num * 10 + ((*c.chr) - '0');

        } else if (
            isutf8whitespace(c.chr)  ||
            utf8cmp(c.chr, "}") == 0 ||
            utf8cmp(c.chr, ",") == 0
        ) {
            return_char(stream, c);
            break;

        } else if (c.len == 1 && *c.chr < 0) {
            parse_state = PS_ERROR;
            parse_error = PE_MISSING_BRACKET;
            break;

        } else {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            break;
        }
    }

    return num;
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
 * Sets parse_error flag on error.
 */
Object create_object(FILE *stream) {
    utf8char c;
    Object out = (Object){
        .relation_count = 0,
        .relations = malloc(sizeof(Relation *))
    };
    bool allow_comma = false;

    while (!feof(stream)) {
        c = get_char(stream);

        if (utf8cmp(c.chr, "\"") == 0) {
            return_char(stream, c);

            Relation rel = create_relation(stream);
            if (parse_state != PS_OK) {
                return out;
            }

            out.relation_count++;
            Relation *r = realloc(out.relations, out.relation_count * sizeof(Relation));
            assert(r != NULL && "Error allocating memory for relation.");
            r[out.relation_count - 1] = rel;
            out.relations = r;
            allow_comma = true;

        } else if (utf8cmp(c.chr, "}") == 0) {
            break;

        } else if (utf8cmp(c.chr, ",") == 0) {
            if (allow_comma) {
                allow_comma = false;
            } else {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return out;
            }

        } else if (utf8cmp(c.chr, "]") == 0) {
            parse_state = PS_ERROR;
            parse_error = PE_MISSING_BRACKET;
            return out;

        } else if (!isutf8whitespace(c.chr)) {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            return out;
        }
    }

    // TODO: check for empty object?

    parse_state = PS_OK;
    return out;
}

/*
 * Parses a stream of characters to create a relation.
 */
Relation create_relation(FILE *stream) {
    Relation r = (Relation){.value_type = -1};

    enum TokenType last_token = TOK_NON;

    while (!feof(stream)) {
        utf8char uc = get_char(stream);
        char *c = uc.chr;

        if (utf8cmp(c, "\"") == 0) {
            if (last_token != TOK_NON && last_token != TOK_DC) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            }

            String str = create_string(stream);
            if (parse_state != PS_OK) {
                return r;
            }

            if (last_token == TOK_NON) {
                r.key = str;
            } else if (last_token == TOK_DC) {
                r.value_type = VALUE_STR;
                r.value.str = str;
            }

            last_token = TOK_STR;

        } else if (utf8cmp(c, ":") == 0) {
            if (last_token == TOK_DC || r.value_type != -1) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            }

            last_token = TOK_DC;

        } else if (utf8cmp(c, "{") == 0) {
            assert(0 && "nested objects not implemented.");

        } else if (utf8cmp(c, "[") == 0) {
            List *l = create_list(stream);

            if (parse_state != PS_OK) {
                return r;
            }

            r.value_type = VALUE_LIST;
            r.value.list = l;
            last_token = TOK_LIST;

        } else if (utf8cmp(c, "}") == 0 || utf8cmp(c, ",") == 0 ) {
            return_char(stream, uc);
            break;

        } else if (isutf8whitespace(c)) {
            ; // ignore whitespace

        } else if (uc.len == 1 && *c < 0) {
            parse_state = PS_ERROR;
            parse_error = PE_MISSING_BRACKET;
            return r;

        } else if (uc.len == 1 && isdigit(*c)) {
            if (last_token != TOK_DC) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return r;
            }

            return_char(stream, uc);
            r.value.num = create_number(stream);
            if (parse_state != PS_OK) {
                return r;
            }

            r.value_type = VALUE_NUM;
            last_token = TOK_NUM;

        } else {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            return r;
        }
    }

    if (r.value_type == -1) {
        parse_state = PS_ERROR;
        parse_error = PE_MISSING_VALUE;

    } else {
        parse_state = PS_OK;
    }

    return r;
}

List *create_list(FILE *stream) {
    utf8char c;
    List l = (List){ .object_count = 0 };
    bool allow_comma = false;

    while (!feof(stream)) {
        c = get_char(stream);

        if (utf8cmp(c.chr, "{") == 0) {
            Object obj = create_object(stream);

            if (parse_state != PS_OK) {
                return NULL;
            }

            if (l.object_count == 0) {
                new_list(&l);
            }

            l.object_count++;
            Object *o = realloc(l.elements, l.object_count * sizeof(Object));
            assert(o != NULL && "Error allocating memory for Object");
            o[l.object_count - 1] = obj;
            l.elements = o;
            allow_comma = true;

        } else if (utf8cmp(c.chr, ",") == 0) {
            if (allow_comma) {
                allow_comma = false;
            } else {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return NULL;
            }

        } else if (
            utf8cmp(c.chr, "}") == 0 ||
            (c.len == 1 && *c.chr < 0)
            ) {
            parse_state = PS_ERROR;
            parse_error = PE_MISSING_BRACKET;
            return NULL;

        } else if (utf8cmp(c.chr, "]") == 0) {
            break;

        } else if (!isutf8whitespace(c.chr)) {
            parse_state = PS_ERROR;
            parse_error = PE_INVALID_CHAR;
            return NULL;
        }

    }

    List *out = malloc(sizeof(List));
    if (out == NULL) {
        printf("Fatal error: Can't malloc parsed list.\n");
        exit(1);
    }
    *out = l;

    parse_state = PS_OK;
    return out;
}
