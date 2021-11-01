#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "common.h"
#include "parse.h"

static char *trim(char *s, size_t len) {
    char tmp[len + 1];
    size_t i;

    for (i = 0; i < len; ++i) { // remove left spaces
        if (s[i] != ' ') {
            memset(tmp, 0, (len + 1) * sizeof(char));
            strcpy(tmp, s + i);
            break;
        }
    }

    for (i = strlen(tmp) - 1; i > 0; --i) { // remove right spaces/newlines
        if (tmp[i] != ' ' && tmp[i] != '\n') {
            memset(s, 0, (len + 1) * sizeof(char));
            strncpy(s, tmp, i + 1);
            break;
        }
    }

    return realloc(s, (strlen(s) + 1) * sizeof(char));
}

static char *extract_line_content(char *line, size_t line_len) {
    char *out = malloc(sizeof(char) * (line_len + 1));
    strncpy(out, line, line_len);
    out[line_len] = 0;

    return trim(out, line_len);
}

static char *extract_file_content(FILE *file, char delim, bool consume_delim) {
    fgetc(file);
    size_t char_count = 1;
    while (!feof(file)) {
        if (fgetc(file) == delim) {
            fseek(file, -1, SEEK_CUR);
            break;
        }
        char_count++;
    }
    fseek(file, -char_count, SEEK_CUR);

    char *text = malloc(sizeof(char) * (char_count + 1));
    for (size_t i = 0; i < char_count; ++i) {
        text[i] = fgetc(file);
    }
    text[char_count] = 0;
    if (consume_delim) fgetc(file);

    return trim(text, char_count);
}

static size_t parse_id(char *str) {
    size_t num = 0;

    for (size_t i = 0; i < strlen(str); ++i) {
        num *= 10;
        num += str[i] - '0';
    }

    return num;
}

static bool parse_options(char *str, struct Sec *s) {
    char *token = strtok(str, "\n");
    size_t option_count = 0;
    struct Opt options[MAX_OPTION_COUNT];

    while (token != NULL) {
        struct Opt op;
        parse_option(token, &op);
        options[option_count] = op;
        option_count++;

        token = (strtok(NULL, "\n"));
    }

    if (option_count != 0) {
        s->options = malloc(sizeof(struct Opt) * option_count);
        memcpy(s->options, options, sizeof(struct Opt) * option_count);
        s->opt_count = option_count;
    }

    return true;
}

static bool parse_section(FILE *file, struct Sec *out) {
    while (!feof(file)) {
        char c = fgetc(file);
        if (c == '<') {
            out->id = parse_id(extract_file_content(file, '>', false));
        } else if (c == '>') {
            out->text = extract_file_content(file, '[', false);
        } else if (c == '[') {
            parse_options(extract_file_content(file, ']', true), out);
            break;
        } else if (c == ' ' || c == '\t' || c == '\n') {
            ;
        } else { // invalid character somewhere.
            return false;
        }
    }
    return true;
}

bool parse_sections(FILE *file, size_t count, struct Adventure *a) {
    struct Sec *sections = malloc(sizeof(struct Sec) * count);

    for (size_t i = 0; i < count; ++i) {
        struct Sec s;
        parse_section(file, &s);
        sections[s.id] = s;
    }

    a->sections = sections;
    return true;
}

bool parse(FILE *file, struct Adventure *a) {
    size_t len;
    char *ptr;

    ptr = fgetln(file, &len);
    if (ptr == NULL) return false;
    a->title = extract_line_content(ptr, len);

    ptr = fgetln(file, &len);
    if (ptr == NULL) return false;
    a->author = extract_line_content(ptr, len);

    ptr = fgetln(file, &len);
    if (ptr == NULL) return false;
    a->version = extract_line_content(ptr, len);

    ptr = fgetln(file, &len);
    if (ptr == NULL) return false;
    ptr = extract_line_content(ptr, len);
    size_t count = strtol(ptr, &ptr, 10);

    parse_sections(file, count, a);

    return true;
}
