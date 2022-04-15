#ifndef TEXT_ADVENTURES_PARSE
#define TEXT_ADVENTURES_PARSE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "utf8.h"

typedef struct utf8char {
    char *chr;
    size_t len; // includes null char
} utf8char;

typedef struct String {
    char *chars;
    size_t len; // includes null char
} String;

enum ValueEnum {
    VALUE_NUM,
    VALUE_STR,
    VALUE_LIST
};

struct ObjectList;

typedef union Value {
    size_t num;
    String str;
    struct ObjectList *list;
} Value;

typedef struct Relation {
    String key;
    Value value;
    enum ValueEnum value_type;
} Relation;

typedef struct Object {
    size_t relation_count;
    Relation *relations;
} Object;

typedef struct ObjectList {
    size_t object_count;
    Object *elements;
} List;

// --------------------------------------------------------

typedef struct Section {
    char *text;
    size_t id;
    size_t option_count;
    struct Option *options;
} Section;

typedef struct Option {
    char *text;
    size_t section_id;
} Option;

typedef struct Adventure {
    char *title;
    char *author;
    char *version;
    size_t section_count;
    struct Section *current_section;
    struct Section *sections;
} Adventure;

// --------------------------------------------------------

#define MAX_SECTION_TEXT_CHAR_LIMIT 300
#define MAX_OPTION_TEXT_CHAR_LIMIT 80
#define MAX_OPTION_COUNT 5
#define MAX_NUMERIC_VALUE 0x186A0 // 100,000 decimal


enum ParseStateEnum {
    PS_OK,
    PS_ERROR,
    PS_UNREACHABLE,
};

enum ParseErrorEnum {
    // JSON to Object
    PE_EMPTY_FILE,
    PE_INVALID_CHAR,
    PE_MISSING_VALUE,
    PE_NUMBER_TOO_BIG,
    PE_MISSING_BRACKET,
    PE_MISSING_DOUBLE_QUOTES,

    // Object to Adventure
    PE_REPEATED_KEY,
    PE_INVALID_KEY,
    PE_MISSING_KEY,
    PE_NO_SECTIONS,
};

// --------------------------------------------------------
// parse flags, set by json_parse
enum ParseStateEnum parse_state;
enum ParseErrorEnum parse_error;
size_t p_col, p_row, p_prev_col;

// --------------------------------------------------------

Object json_parse(FILE *stream);
Adventure json_to_adventure(Object adventure);

#endif // TEXT_ADVENTURES_PARSE
