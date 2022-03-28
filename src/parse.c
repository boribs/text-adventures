#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utf8.h"
#include "parse.h"

// TODO: keep track of column and row

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

/*
 * Takes a stream of characters and interprets it  as JSON.
 * Sets parse_error flag on error.
 */
Object json_parse(FILE *stream) {
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
    Object out = (Object){};
    String s = (String){.len = 1};
    Relation r = (Relation){};
    bool expecting_value = false,
         expecting_double_colon = false,
         is_string = false;

    while (!feof(stream)) {
        c = get_char(stream);

        if (utf8cmp(c.chr, "\"") == 0) {
            // begin / end string
            // remember it can be escaped!
        } else if (isutf8whitespace(c.chr)) {
            if (is_string) {
                charcat(&s, &c);
            } else if (!expecting_value) {
                expecting_double_colon = true;
            }
        } else if (utf8cmp(c.chr, ":") == 0) {
            if (is_string) {
                charcat(&s, &c);
            } else {
                r.key = s;
                expecting_value = true;
                s = (String){};
            }
        } else if (utf8cmp(c.chr, "{") == 0) {
            // check if expecting value, create object / add to string
        } else if (utf8cmp(c.chr, "[") == 0) {
            // check if expecting value, create list
        } else if (*c.chr > 0) {
            if (expecting_double_colon) {
                parse_state = PS_ERROR;
                parse_error = PE_INVALID_CHAR;
                return out;
            }

            charcat(&s, &c);
        } else {
            // got to the end without closing the object
            break;
        }

        if (utf8cmp(c.chr, "}") == 0) {
            break;
        }
    }

    // TODO: check for empty object?
    // TODO: check for incomplete object

    if (expecting_value) {
        parse_state = PS_ERROR;
        parse_error = PE_MISSING_VALUE;

        return out;
    }

    parse_state = PS_OK;
    return out;
}
