#ifndef TEXT_ADVENTURES_COMMON
#define TEXT_ADVENTURES_COMMON

#include <stdlib.h>

#define SEC_TEXT_CHAR_LIMIT 300
#define OPT_TEXT_CHAR_LIMIT 80
#define MAX_OPTION_COUNT 5

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
    size_t sec_count;
    struct Sec *sections;
};

#endif // TEXT_ADVENTURES_COMMON
