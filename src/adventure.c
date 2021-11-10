#include <stdio.h>
#include <stdbool.h>

#include "common.h"
#include "parse.h"
#include "adventure.h"

static void show_parse_state(enum ParseState p) {
    switch(p) {
        case P_STATE_OK: P_STATE_UNREACHABLE: break;
        case P_STATE_INVALID_CHARACTER_OPENING_ID_DEL:
            printf("Can't have `<` inside ID tag.\n");break;
        case P_STATE_INVALID_CHARACTER_CLOSING_ID_DEL:
            printf("Can't have `>` outside ID tag.\n");break;
        case P_STATE_INVALID_CHARACTER_OPENING_OPTION_DEL:
            printf("Can't have `[` in ID tag.\n");break;
        case P_STATE_INVALID_CHARACTER_CLOSING_OPTION_DEL:
            printf("Can't have `]` in ID tag.\n");break;
        case P_STATE_INVALID_CHAR_IN_ID:
            printf("Found invalid character in ID\n");break;
        case P_STATE_INVALID_LAST_TOKEN:
            printf("The last thing in the file should be `]`\n");break;
        case P_STATE_MISSING_ADVENTURE_DATA:
            printf("The first lines in the file should be: title, author, version\n");break;
        case P_STATE_MISSING_AUTHOR:
            printf("Author not provided.\n");break;
        case P_STATE_MISSING_VERSION:
            printf("Version not provided.\n");break;
        case P_STATE_INVALID_SYNTAX_EXPECTED_TEXT:
            printf("Expected text, got something else.\n");break;
        case P_STATE_INVALID_SYNTAX_EXPECTED_ID:
            printf("Expected ID, got something else.\n");break;
        case P_STATE_TOO_MANY_OPTIONS_IN_SECTION:
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

    enum ParseState p = parse(f, a);
    show_parse_state(p);
    if (p != P_STATE_OK) return false;

    fclose(f);
    a->current_section = &a->sections[0];

    return true;
}

static void show_adventure_data(struct Adventure *a) {
    printf("%s\n", a->title);
    printf("by %s\n", a->author);
    printf("%s\n\n", a->version);
}

static void show_option(size_t num, struct Opt *o) {
    printf("%zu) %s\n", num + 1, o->text);
}

static void show_section(struct Sec *s) {
    printf("%s\n\n\n", s->text);

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
    struct Adventure a;

    if (!load_adventure(filename, &a)) {
        exit(1);
    }

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
