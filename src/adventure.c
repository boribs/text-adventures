#include <stdio.h>
#include <stdbool.h>

#include "common.h"
#include "parse.h"

bool load_adventure(char *filename, struct Adventure *a) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) return false;
    if (!parse(f, a)) return false;
    fclose(f);
    a->current_section = &a->sections[0];

    return true;
}

void show_adventure_data(struct Adventure *a) {
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

void show_current_section(struct Adventure *a) {
    show_section(a->current_section);
}

static void goto_section(struct Adventure *a, size_t i) {
    a->current_section = &a->sections[a->current_section->options[i - 1].sec_id];
    show_section(a->current_section);
}

bool end_of_adventure(struct Adventure *a) {
    return a->current_section->opt_count == 0;
}

static bool is_valid_input(int input, struct Adventure *a) {
    return (
        input - 1 >= 0 &&
        input - 1 <= a->current_section->opt_count
    );
}

void get_input(struct Adventure *a) {
    int input;
    do {
        fflush(stdin);
        printf("> ");
        scanf("%d", &input);
    } while(!is_valid_input(input, a));

    printf("\n");
    goto_section(a, input);
}
