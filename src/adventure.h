#ifndef TEXT_ADVENTURES_ADVENTURE
#define TEXT_ADVENTURES_ADVENTURE

#include <stdbool.h>

#define KEY_QUIT 'q'

bool load_adventure(char *filename, struct Adventure *a);
void show_adventure_data(struct Adventure *a);
void show_current_section(struct Adventure *a);
bool end_of_adventure(struct Adventure *a);
bool get_input(struct Adventure *a);

#endif // TEXT_ADVENTURES_ADVENTURE
