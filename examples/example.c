#include <stdio.h>
#include "../src/common.h"
#include "../src/adventure.h"

#define ADVENTURE_FILE "adventurefiles/veryshortadventure.adv"

int main() {
    play_adventure(ADVENTURE_FILE);

    return 0;
}
