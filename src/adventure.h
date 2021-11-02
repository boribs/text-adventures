#ifndef TEXT_ADVENTURES_ADVENTURE
#define TEXT_ADVENTURES_ADVENTURE

#include <stdbool.h>

bool load_adventure(char *filename, struct Adventure *a);
void show_adventure_data(struct Adventure *a);
void show_current_section(struct Adventure *a);
bool end_of_adventure(struct Adventure *a);
void get_input(struct Adventure *a);

#endif // TEXT_ADVENTURES_ADVENTURE
