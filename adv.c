#include <stdio.h>

#include "src/adventure.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No input file!\n");
        return 1;
    }

    play_adventure(*(argv + 1));
    return 0;
}
