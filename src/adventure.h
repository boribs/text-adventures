// stuff related to display and ui

#ifndef TEXT_ADVENTURES_ADVENTURE
#define TEXT_ADVENTURES_ADVENTURE

#include <stdbool.h>

enum InputOptions { // as many as MAX_OPTION_COUNT (common.h)
    ADVENTURE_INPUT_OPTION_1,
    ADVENTURE_INPUT_OPTION_2,
    ADVENTURE_INPUT_OPTION_3,
    ADVENTURE_INPUT_OPTION_4,
    ADVENTURE_INPUT_OPTION_5,
    ADVENTURE_INPUT_QUIT,
    ADVENTURE_INPUT_INVALID,
};

#define KEY_QUIT 'q'
#define L_O_PADDING 1
#define R_O_PADDING 1
#define L_I_PADDING 1
#define R_I_PADDING 1

static const int O_PADDING = L_O_PADDING + R_O_PADDING;
static const int I_PADDING = L_I_PADDING + R_I_PADDING;
static const int L_PADDING = L_O_PADDING + L_I_PADDING;
static const int R_PADDING = R_O_PADDING + R_I_PADDING;

void play_adventure(char *filename);

#endif // TEXT_ADVENTURES_ADVENTURE
