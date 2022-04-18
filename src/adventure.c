#include <stdio.h>
#include <stdbool.h>

#include "parse.h"
#include "adventure.h"

static void show_error_message() {}

static enum InputType get_input() {
    fflush(stdin);
    scanf("%c", &input);

    if (input == 'q' || input == 'Q') {
        return ADVENTURE_INPUT_QUIT;
    } else if (input > '0' && input < '9') {
        return ADVENTURE_INPUT_OPTION;
    } else {
        return ADVENTURE_INPUT_INVALID;
    }
}

static bool play_section(Adventure *adv) {
    printf("%s\n\n", adv->current_section->text);

    if (adv->current_section->option_count == 0) {
        return true;
    }

    for (size_t i = 0; i < adv->current_section->option_count; ++i) {
        printf("%zu) %s\n", i + 1, adv->current_section->options[i].text);
    }

    enum InputType input_type;
    do {
        input_type = get_input();
    } while (input_type == ADVENTURE_INPUT_INVALID);

    if (input_type == ADVENTURE_INPUT_QUIT) {
        return true;
    }

    input -= 49;
    size_t next_id = adv->current_section->options[input].section_id;

    for (size_t i = 0; i < adv->section_count; ++i) {
        if (adv->sections[i].id == next_id) {
            adv->current_section = &adv->sections[i];
            break;
        }
    }

    return false;
}

void play_adventure(char *filename) {
    FILE *f = fopen(filename, "r");

    if (f == NULL) {
        printf("File not found!\n");
        return;
    }

    Adventure adv = json_to_adventure(json_parse(f));
    fclose(f);

    if (parse_state != PS_OK) {
        show_error_message();
        return;
    }

    adv.current_section = adv.sections; // start at first section
    while (!play_section(&adv));
}
