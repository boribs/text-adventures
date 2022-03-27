#include <stdio.h>
#include <stdlib.h>

#include "utf8.h"
#include "parse.h"

// why tf does this not work???
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
 * Takes a stream of characters and interprets it  as JSON.
 * Sets parse_error flag on error.
 */
Object json_parse(FILE *stream) {
    utf8char c;

    while (!feof(stream)) {
        c = get_char(stream);

        if (*c.chr < 0) break;

        if (utf8cmp(c.chr, "{") == 0) {
            parse_state = PS_OK;
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
 */
Object create_object(FILE *stream) {
    return (Object){};
}
