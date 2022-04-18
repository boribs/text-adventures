#include <stdio.h>
#include "../src/adventure.h"

#define ADVENTURE_FILE "tests/test_file_bigger_adventure.json"

int main() {
    play_adventure(ADVENTURE_FILE);

    return 0;
}
