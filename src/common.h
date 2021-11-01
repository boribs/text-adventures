#ifndef TEXT_ADVENTURES_COMMON
#define TEXT_ADVENTURES_COMMON

#include <stdlib.h>

struct Sec {
    char *text;
    size_t id;
    size_t opt_count;
    struct Opt *options;
};

struct Opt {
    char *text;
    size_t sec_id;
};

struct Adventure {
    char *title;
    char *author;
    char *version;
    struct Sec *sections;
};

#endif // TEXT_ADVENTURES_COMMON
