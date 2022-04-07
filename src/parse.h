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

#define MAX_SECTION_TEXT_CHAR_LIMIT 300
#define MAX_OPTION_TEXT_CHAR_LIMIT 80
#define MAX_OPTION_COUNT 5
#define MAX_NUMERIC_VALUE 10000


enum ParseStateEnum {
    PS_OK,
    PS_ERROR,
    PS_UNREACHABLE,
};

enum ParseErrorEnum {
    PE_EMPTY_FILE,
    PE_INVALID_CHAR,
    PE_MISSING_VALUE,
    PE_NUMBER_TOO_BIG,
    PE_MISSING_BRACKET,
    PE_MISSING_DOUBLE_QUOTES,
};

// --------------------------------------------------------
// parse flags, set by json_parse
enum ParseStateEnum parse_state;
enum ParseErrorEnum parse_error;
size_t p_col, p_row;

// --------------------------------------------------------

Object json_parse(FILE *stream);
Object create_object(FILE *stream); // make static
Relation create_relation(FILE *stream); // make static
utf8char get_char(FILE *stream); // make static

#endif // TEXT_ADVENTURES_PARSE
