#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#include "utf8.h"
#include "parse.h"
#include "adventure.h"

struct winsize w;
int col = 0; // cursor position
struct termios t;

enum Delimiter {
    WT_NEWLINE,
    WT_BLANK,
    WT_NONE,
};

void show_error_message() {}

static void print_l_border() {
    for (size_t i = 0; i < L_O_PADDING; ++i) printf(" ");
    printf("|");
    for (size_t i = 0; i < L_I_PADDING; ++i) printf(" ");
    col = L_PADDING + 1;
}

static void complete_line(bool nl) {
    size_t const max_col = w.ws_col - R_O_PADDING - 1;
    for (int i = col; i < max_col; ++i) printf(" ");
    printf("|");
    if (nl) printf("\n");
}

static void print_empty_line() {
    print_l_border();
    complete_line(true);
}

static void print_full_border() {
    for (size_t i = 0; i < L_O_PADDING; ++i) printf(" ");
    printf("+");

    size_t const max_col = w.ws_col - R_O_PADDING - 1;
    for (int i = L_I_PADDING + 1; i < max_col; ++i) printf("-");
    printf("+\n");
}

static enum Delimiter get_word(char *dest, size_t *dest_len, char *src, size_t *del_len) {
    char pool[100] = {0};
    *dest_len = 0;

    while (true) {
        size_t i = 1;
        while (utf8nvalid(src, i) != 0) { ++i; }
        utf8ncat(pool, src, i);

        if (*pool == 0) {
            return WT_NONE;
        }

        if (isutf8whitespace(pool)) {
            *del_len = i;
            if (*pool == '\n') {
                return WT_NEWLINE;
            } else {
                return WT_BLANK;
            }
        }

        utf8cat(dest, pool);
        memset(pool, 0, 100);
        (*dest_len)++;
        src += i;
    }
}

static void print_text(char *text) {
    char word[100] = {0};
    size_t word_len = 0, delimiter_len;
    size_t const max_col = w.ws_col - R_PADDING - 1;

    if (col == 0) {
        print_l_border();
    }

    enum Delimiter s = WT_BLANK;
    while (*text && s != WT_NONE) {
        s = get_word(word, &word_len, text, &delimiter_len);

        if (col + word_len < max_col) {
            printf("%s", word);
            col += word_len;
        } else {
            complete_line(true);
            print_l_border();
            printf("%s", word);
            col += word_len;
        }

        if (col + delimiter_len < max_col) {
            if (s == WT_NEWLINE) {
                printf("\n");
                print_l_border();
            } else {
                printf(" ");
                col++;
            }
        }

        text += strlen(word) + delimiter_len;
        memset(word, 0, 100);
    }

    complete_line(false);
}

static enum InputType get_input() {
    fflush(stdin);
    scanf("%c", &input);

    if (input == 'q' || input == 'Q') {
        printf("%c", input);
        col += 1;
        complete_line(true);
        print_full_border();
        return ADVENTURE_INPUT_QUIT;

    } else if (input > '0' && input < '9') {
        printf("%c", input);
        col += 1;
        complete_line(true);
        print_empty_line();
        print_l_border();
        return ADVENTURE_INPUT_OPTION;

    } else {
        return ADVENTURE_INPUT_INVALID;
    }
}

static bool play_section(Adventure *adv) {
    print_text(adv->current_section->text);
    printf("\n");

    if (adv->current_section->option_count == 0) {
        print_full_border();
        return true;
    }

    print_l_border();
    complete_line(false);

    for (size_t i = 0; i < adv->current_section->option_count; ++i) {
        printf("\n");
        print_l_border();
        printf("%zu) ", i + 1);
        col += 3;
        print_text(adv->current_section->options[i].text);
    }
    printf("\n");
    print_l_border();
    printf("> ");
    col += 2;

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

    tcgetattr(0, &t);
    t.c_lflag &= ~(ECHO|ICANON);
    tcsetattr(0, TCSANOW, &t);

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); // get terminal size

    adv.current_section = adv.sections;   // start at first section
    print_full_border();
    while (!play_section(&adv));          // main loop
}
