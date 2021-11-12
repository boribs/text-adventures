#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "common.h"
#include "parse.h"
#include "adventure.h"

struct winsize w; // terminal size
int col = 0; // cursor position

static void show_error_message(char *filename, struct TokenError terr) {
    if (terr.state == P_STATE_OK) return;

    switch(terr.state) {
        case P_STATE_UNREACHABLE: break;
        case P_STATE_INVALID_CHARACTER_OPENING_ID_DEL:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Can't have `<` inside ID tag.\n");break;
        case P_STATE_INVALID_CHARACTER_CLOSING_ID_DEL:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Can't have `>` outside ID tag.\n");break;
        case P_STATE_MISSING_ID_NUMBER:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("IDs must have a number: for example <3>.\n");break;
        case P_STATE_INVALID_CHARACTER_OPENING_OPTION_DEL:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Can't have `[` in ID tag.\n");break;
        case P_STATE_INVALID_CHARACTER_CLOSING_OPTION_DEL:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Can't have `]` in ID tag.\n");break;
        case P_STATE_INVALID_CHAR_IN_ID:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Found invalid character in ID\n");break;
        case P_STATE_INVALID_LAST_TOKEN:
            printf("The last thing in the file should be `]`. Unclosed section?\n");break;
        case P_STATE_MISSING_ADVENTURE_DATA:
            printf("The first lines in the file should be: title, author, version\n");break;
        case P_STATE_MISSING_AUTHOR:
            printf("Author not provided.\n");break;
        case P_STATE_MISSING_VERSION:
            printf("Version not provided.\n");break;
        case P_STATE_INVALID_SYNTAX_EXPECTED_TEXT:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Expected text, got something else.\n");break;
        case P_STATE_INVALID_SYNTAX_EXPECTED_ID:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Expected ID, got something else.\n");break;
        case P_STATE_TOO_MANY_OPTIONS_IN_SECTION:
            printf("%s:%zu:%zu: ", filename, terr.row, terr.col);
            printf("Section has too many options. The maximum is 5.\n");break;
        case P_STATE_NO_SECTIONS_IN_ADVENTURE:
            printf("The adventure should have at least one section!\n");break;
        default:
            break; // unreachable
    }
}

static bool load_adventure(char *filename, struct Adventure *a) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;

    struct TokenError e = parse(f, a);
    show_error_message(filename, e);
    if (e.state != P_STATE_OK) return false;

    fclose(f);
    a->current_section = &a->sections[0];

    return true;
}

static void print_n_chars(char *str, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        printf("%c", *(str + i));
    }
}

static void print_boxed_text(char *str, int trailing_nl) {
    char text[strlen(str) + 1], del;
    strcpy(text, str);
    size_t tok_len;

    char *tok = strtok(text, " \n");
    while (tok != NULL) {
        tok_len = strlen(tok);
        del = str[tok - text + tok_len];

        if (tok_len < w.ws_col - col) {
            printf("%s", tok);
        } else {
            printf("\n%s", tok);
            col = 0;
        }

        if (del == ' ') {
            printf(" ");
            col++;
        } else if (del == '\n') {
            printf("\n");
            col = 0;
        }

        col += tok_len;
        tok = strtok(NULL, " \n");
    }

    for (int i = 0; i < trailing_nl; ++i) {
        col = 0;
        printf("\n");
    }
}

static void show_adventure_data(struct Adventure *a) {
    print_boxed_text(a->title, 1);
    print_boxed_text(a->author, 1);
    print_boxed_text(a->version, 2);
}

static void show_option(size_t num, struct Opt *o) {
    col = 3;
    printf("%zu) ", num + 1);
    print_boxed_text(o->text, 1);
}

static void show_section(struct Sec *s) {
    print_boxed_text(s->text, 2);

    for (size_t i = 0; i < s->opt_count; ++i) {
        show_option(i, &s->options[i]);
    }
    printf("\n");
}

static void show_current_section(struct Adventure *a) {
    show_section(a->current_section);
}

static void goto_section(struct Adventure *a, enum InputOptions i) {
    size_t id = a->current_section->options[i].sec_id;

    for (size_t i = 0; i < a->sec_count; ++i) {
        if (a->sections[i].id == id) {
            a->current_section = &a->sections[i];
            break;
        }
    }
    show_section(a->current_section);
}

static bool end_of_adventure(struct Adventure *a) {
    return a->current_section->opt_count == 0;
}

static enum InputOptions validate_input(char input, struct Adventure *a) {
    int i = input - '0' - 1;

    if (i >= 0 && i <= a->current_section->opt_count) {
        return (enum InputOptions)i;
    } else if (input == KEY_QUIT) {
        return ADVENTURE_INPUT_QUIT;
    } else {
        return ADVENTURE_INPUT_INVALID;
    }
}

static enum InputOptions get_input(struct Adventure *a) {
    char input;
    enum InputOptions i;

    do {
        fflush(stdin);
        printf("> ");
        scanf("%c", &input);
        i = validate_input(input, a);
    } while(i == ADVENTURE_INPUT_INVALID);

    printf("\n");
    return i;
}

void play_adventure(char *filename) {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // get terminal size

    struct Adventure a;

    if (!load_adventure(filename, &a)) {
        exit(1);
    }

    // system("clear");
    show_adventure_data(&a);
    show_current_section(&a);

    while (!end_of_adventure(&a)) {
        enum InputOptions input = get_input(&a);

        if (input == ADVENTURE_INPUT_QUIT) {
            return;
        } else if (input == ADVENTURE_INPUT_INVALID) {
            ; // unreachable
        } else {
            goto_section(&a, input);
        }
    }
}
