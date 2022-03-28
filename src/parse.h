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
    size_t len;
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

// #define PARSER_IGNORE_TRAILING_COMMAS
// #define PARSER_TRY_TO_PARSE_WHEN_LAST_BRACKET_MISSING
#define WARN_SECTION_TEXT_CHAR_LIMIT 300
#define WARN_OPTION_TEXT_CHAR_LIMIT 80
#define WARN_MAX_OPTION_COUNT 5


enum ParseStateEnum {
    PS_OK,
    PS_UNREACHABLE,
    PS_ERROR
};

enum ParseErrorEnum {
    PE_EMPTY_FILE,
    PE_INVALID_CHAR,
    PE_MISSING_BRACKET,
    PE_MISSING_DOUBLE_QUOTES,
};

// --------------------------------------------------------
// parse flags, set by json_parse
enum ParseStateEnum parse_state;
enum ParseErrorEnum parse_error;
size_t err_col, err_row;

// --------------------------------------------------------

Object json_parse(FILE *stream);
Object create_object(FILE *stream); // make static
utf8char get_char(FILE *stream); // make static

#endif // TEXT_ADVENTURES_PARSE
