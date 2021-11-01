#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdbool.h>

bool parse(FILE *file, struct Adventure *a);
bool parse_sections(FILE *file, size_t count, struct Adventure *a);

#endif // TEXT_ADVENTURES_PARSE
