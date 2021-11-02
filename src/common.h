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
    struct Sec *current_section;
    struct Sec *sections;
};

static void clear_opt(struct Opt *o) {
    free(o->text);
}

static void clear_sec(struct Sec *s) {
    free(s->text);
    for (size_t i = 0; i < s->opt_count; ++i) {
        clear_opt(&s->options[i]);
    }
    free(s->options);
}

static void clear_adventure(struct Adventure *a) {
    free(a->title);
    free(a->author);
    free(a->version);
    for (size_t i = 0; i < a->sec_count; ++i) {
        clear_sec(&a->sections[i]);
    }
}

#endif // TEXT_ADVENTURES_COMMON
