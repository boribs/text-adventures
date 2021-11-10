#include <stdio.h>
#include "../../src/common.h"
#include "../../src/adventure.h"

int main() {
    struct Adventure a;
    if (!load_adventure("adventure.adv", &a)) {
        printf("Error opening adventure\n");
        return 1;
    }

    show_adventure_data(&a);
    show_current_section(&a);

    while (!end_of_adventure(&a) && get_input(&a)) {
        ;
    }

    return 0;
}
