#include <stdio.h>
#include "../src/common.h"
#include "../src/adventure.h"

#define ADVENTURE_FILE "adventurefiles/veryshortadventure.adv"
// #define ADVENTURE_FILE "adventurefiles/abc.adv"

int main() {
    play_adventure(ADVENTURE_FILE);

    return 0;
}
