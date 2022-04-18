#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "unity/unity.h"
#include "../src/parse.h"

FILE *stream;
char *buffer;

void setUp() {
    stream = NULL;
    buffer = NULL;
}

void tearDown() {
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }
    if (stream != NULL) {
        fclose(stream);
        stream = NULL;
    }
}

#define S &s[0]
#define TEST_ASSERT_STATE(ps)         TEST_ASSERT_EQUAL(ps, parse_state)
#define TEST_ASSERT_ERROR(pe)         TEST_ASSERT_EQUAL(pe, parse_error)
#define TEST_ASSERT_NO_ERROR()        TEST_ASSERT_STATE(PS_OK)
#define TEST_ASSERT_POSITION(r, c)    TEST_ASSERT_EQUAL(r, p_row); \
                                      TEST_ASSERT_EQUAL(c, p_col)

static Relation SRel(char *key, char *val) {
    return (Relation){
        .key = (String){ .chars = key, .len = utf8len(key) + 1 },
        .value_type = VALUE_STR,
        .value.str = (String){ .chars = val, .len = utf8len(val) + 1 }
    };
}

static Relation NRel(char *key, size_t val) {
    return (Relation){
        .key = (String){ .chars = key, .len = utf8len(key) + 1 },
        .value_type = VALUE_NUM,
        .value.num = val
    };
}

static void construct_file_like_obj(char *s) {
    size_t ssize = strlen(s) + 1;
    buffer = malloc(sizeof(char) * ssize);
    stream = fmemopen(buffer, ssize, "w+");
    fprintf(stream, "%s", s);
    rewind(stream);
}


// JSON parse tests
static void compare_lists(List *expected, List *actual);

static void compare_strings(String expected, String actual) {
    TEST_ASSERT_EQUAL(0, utf8cmp(expected.chars, actual.chars));
    TEST_ASSERT_EQUAL(expected.len, actual.len);
    TEST_ASSERT_EQUAL(expected.len, actual.len);
}

static void compare_values(Value expected, enum ValueEnum expected_type, Value actual) {
    switch (expected_type) {
        case VALUE_NUM:
            TEST_ASSERT_EQUAL(expected.num, actual.num);
            break;
        case VALUE_STR:
            compare_strings(expected.str, actual.str);
            break;
        case VALUE_LIST:
            compare_lists(expected.list, actual.list);
            break;
        default: {
            char msg[30] = {0};
            sprintf(msg, "Can't be this: %d", expected_type);
            TEST_FAIL_MESSAGE(msg);
            break;
        }
    }
}

static void compare_relations(Relation expected, Relation actual) {
    compare_strings(expected.key, actual.key);

    TEST_ASSERT_EQUAL(expected.value_type, actual.value_type);
    compare_values(expected.value, expected.value_type, actual.value);
}

static void compare_objects(Object expected, Object actual) {
    TEST_ASSERT_EQUAL(expected.relation_count, actual.relation_count);

    for (size_t i = 0; i < actual.relation_count; ++i) {
        compare_relations(expected.relations[i], actual.relations[i]);
    }
}

static void compare_lists(List *expected, List *actual) {
    TEST_ASSERT_EQUAL(expected->object_count, actual->object_count);

    for (size_t i = 0; i < actual->object_count; ++i) {
        compare_objects(expected->elements[i], actual->elements[i]);
    }
}

static void compare_option(Option expected, Option actual) {
    TEST_ASSERT_EQUAL_STRING(expected.text, actual.text);
    TEST_ASSERT_EQUAL(expected.section_id, actual.section_id);
}

static void compare_section(Section expected, Section actual) {
    TEST_ASSERT_EQUAL_STRING(expected.text, actual.text);
    TEST_ASSERT_EQUAL_MESSAGE(expected.id, actual.id, "[id]");
    TEST_ASSERT_EQUAL(expected.option_count, actual.option_count);

    for (size_t i = 0; i < expected.option_count; ++i) {
        compare_option(expected.options[i], actual.options[i]);
    }
}

static void compare_adventures(Adventure expected, Adventure actual) {
    TEST_ASSERT_EQUAL_STRING(expected.title, actual.title);
    TEST_ASSERT_EQUAL_STRING(expected.author, actual.author);
    TEST_ASSERT_EQUAL_STRING(expected.version, actual.version);
    TEST_ASSERT_EQUAL_MESSAGE(expected.section_count, actual.section_count, "[section count]");
    // Ignore current_section, that's used during adventure playthrough

    for (size_t i = 0; i < expected.section_count; ++i) {
        compare_section(expected.sections[i], actual.sections[i]);
    }
}

static void test_json_empty_str(void) {
    construct_file_like_obj("");

    json_parse(stream);
    TEST_ASSERT_ERROR(PE_EMPTY_FILE);
}

static void test_jsom_empty_file_when_only_whitespace_present(void) {
    construct_file_like_obj(" \n");

    json_parse(stream);
    TEST_ASSERT_ERROR(PE_EMPTY_FILE);
}

static void test_json_empty_object(void) {
    construct_file_like_obj("{}");

    Object actual = json_parse(stream);
    Object expected = (Object){};

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_json_empty_mismatched_brackets(void) {
    construct_file_like_obj("}");

    json_parse(stream);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
}

static void test_key_must_be_string(void) {
    construct_file_like_obj("{key:\"value\"}");

    Object actual = json_parse(stream);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 2);
}

static void test_allow_string_with_whitespace_as_key(void) {
    construct_file_like_obj("{\"key with whitespace\":\"val\"}");

    Relation r = SRel("key with whitespace", "val");
    Object actual = json_parse(stream);
    Object expected = (Object){
        .relation_count = 1,
        .relations = &r
    };
    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_escaped_double_quote(void) {
    construct_file_like_obj("{\"this key contains an escaped \\\"\":\"val\"}");

    Object actual = json_parse(stream);

    TEST_ASSERT_NO_ERROR();
    TEST_ASSERT_EQUAL_STRING(
        "this key contains an escaped \"",
        actual.relations[0].key.chars
    );
}

static void test_escaped_unicode_chars(void) {
    construct_file_like_obj("{\"this key contains escaped unicode: \uc3b6\":\"val\"}");

    Object actual = json_parse(stream);

    TEST_ASSERT_NO_ERROR();
    TEST_ASSERT_EQUAL_STRING(
        "this key contains escaped unicode: \uc3b6",
        actual.relations[0].key.chars
    );
}

static void test_object_must_have_name_value_pair(void) {
    construct_file_like_obj("{\"key\":}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_MISSING_VALUE);
}

static void test_object_must_have_name_value_pair2(void) {
    construct_file_like_obj("{\"key\"}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_MISSING_VALUE);
}

static void test_invalid_double_string(void) {
    construct_file_like_obj("{\"invalid\"\"key\":\"valid value\"}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 11);
}

static void test_invalid_double_quote(void) {
    construct_file_like_obj("{\"valid key\":\"valid value\"\"}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 27);
}

static void test_invalid_double_colon(void) {
    construct_file_like_obj("{\"valid\\\"key\"::\"valid value\"}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 15);
}

static void test_invalid_double_colon2(void) {
    construct_file_like_obj("{\"valid\\\"key\":\"valid value\":}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 28);
}

static void test_incomplete_string(void) {
    construct_file_like_obj("{\"key\":\"value");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_MISSING_DOUBLE_QUOTES);
    TEST_ASSERT_POSITION(0, 14);
}

static void test_incomplete_object(void) {
    construct_file_like_obj("{\"key\":\"value\"");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_MISSING_BRACKET);
    TEST_ASSERT_POSITION(0, 15);
}

static void test_relation_with_trailing_comma(void) {
    construct_file_like_obj("{\"key\" : \"value\",}");

    Object actual = json_parse(stream);
    Relation r = SRel("key", "value");
    Object expected = (Object){
        .relation_count = 1,
        .relations = &r
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_relation_with_tow_trailing_commas(void) {
    construct_file_like_obj("{\"key\" : \"value\",,}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 18);
}

static void teset_object_with_two_relations(void) {
    construct_file_like_obj("{\"key1\" : \"value1\", \"key2\" : \"value2\"}");

    Object actual = json_parse(stream);
    Relation rels[2] = {
        SRel("key1", "value1"),
        SRel("key2", "value2"),
    };
    Object expected = (Object){
        .relation_count = 2,
        .relations = rels
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_relation_with_numeric_value(void) {
    construct_file_like_obj("{\"num\" : 123}");

    Object actual = json_parse(stream);
    Relation r = NRel("num", 123);
    Object expected = (Object){
        .relation_count = 1,
        .relations = &r
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_invalid_relation_with_negative_signed_number(void) {
    construct_file_like_obj("{\"num\" : -1}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 10);
}

static void test_invalid_relation_with_positive_signed_number(void) {
    construct_file_like_obj("{\"num\" : +1}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 10);
}

static void test_invalid_relation_with_exponent(void) {
    construct_file_like_obj("{\"num\" : 10e-2}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 12);
}

static void test_invalid_number_with_whitespace(void) {
    construct_file_like_obj("{\"num\" : 54 2}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 13);
}

static void test_number_surpases_limit(void) {
    construct_file_like_obj("{\"num\" : 542234345}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_NUMBER_TOO_BIG);
    TEST_ASSERT_POSITION(0, 15);
}

static void test_incomplete_number(void) {
    construct_file_like_obj("{\"num\" : 1231");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_MISSING_BRACKET);
}

static void test_multiple_numbered_relations(void) {
    construct_file_like_obj("{\"num1\":1,\"num2\":2}");

    Object actual = json_parse(stream);
    Relation rels[2] = {
        NRel("num1", 1),
        NRel("num2", 2),
    };
    Object expected = (Object){
        .relation_count = 2,
        .relations = rels
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_string_and_numbered_relations(void) {
    construct_file_like_obj("{\"num1\":1,\"key2\":\"second key\"}");

    Object actual = json_parse(stream);
    Relation rels[2] = {
        NRel("num1", 1),
        SRel("key2", "second key")
    };
    Object expected = (Object){
        .relation_count = 2,
        .relations = rels
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_empty_list(void) {
    construct_file_like_obj("{\"list\":[]}");

    Object actual = json_parse(stream);

    TEST_ASSERT_NO_ERROR();
    compare_strings(
        (String){ .chars = "list", .len = 5 },
        actual.relations[0].key
    );
    TEST_ASSERT_EQUAL(VALUE_LIST, actual.relations[0].value_type);
    TEST_ASSERT_EQUAL(0, actual.relations[0].value.list->object_count);
    TEST_ASSERT_NULL(actual.relations[0].value.list->elements);
}

static void test_invalid_empty_list(void) {
    construct_file_like_obj("{\"list\":[a]}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 10);
}

static void test_invalid_empty_list_with_separator(void) {
    construct_file_like_obj("{\"list\":[,]}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 10);
}

static void test_list_with_incomplete_object(void) {
    construct_file_like_obj("{\"list\":[{]}");

    json_parse(stream);

    TEST_ASSERT_ERROR(PE_MISSING_BRACKET);
}

static void test_list_with_simple_object_inside(void) {
    construct_file_like_obj("{\"list\":[{\"ando\":\"caminando\"}]}");

    Object actual = json_parse(stream);
    Relation r = SRel("ando", "caminando");
    Object inner = (Object){
        .relation_count = 1,
        .relations = &r
    };
    Object expected = (Object){
        .relation_count = 1,
        .relations = &(Relation){
            .key = (String){ .chars = "list", .len = 5 },
            .value_type = VALUE_LIST,
            .value.list = &(List){ .object_count = 1, .elements = &inner }
        }
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_list_with_two_objects_inside(void) {
    construct_file_like_obj("{\"list\":[{\"ando\":\"caminando\"},{\"con un\":\"flow violento\"}]}");

    Object actual = json_parse(stream);
    Object elems[2];
    Relation r[2] = {
        SRel("ando", "caminando"),
        SRel("con un", "flow violento"),
    };
    elems[0] = (Object){
        .relation_count = 1,
        .relations = &r[0]
    };
    elems[1] = (Object){
        .relation_count = 1,
        .relations = &r[1]
    };

    Object expected = (Object){
        .relation_count = 1,
        .relations = &(Relation){
            .key = (String){ .chars = "list", .len = 5 },
            .value_type = VALUE_LIST,
            .value.list = &(List){ .object_count = 2, .elements = elems }
        }
    };

    TEST_ASSERT_NO_ERROR();
    compare_objects(expected, actual);
}

static void test_file_with_single_section(void) {
    stream = fopen("tests/test_file_with_single_section.json", "r");

    Object actual = json_parse(stream);

    TEST_ASSERT_NO_ERROR();
    TEST_ASSERT_EQUAL(4, actual.relation_count);

    Relation rexp = SRel("title", "first test file!");
    compare_relations(rexp, actual.relations[0]);

    rexp = SRel("author", "me");
    compare_relations(rexp, actual.relations[1]);

    rexp = SRel("version", "1.0");
    compare_relations(rexp, actual.relations[2]);

    TEST_ASSERT_EQUAL(VALUE_LIST, actual.relations[3].value_type);

    Object sec = actual.relations[3].value.list->elements[0];
    TEST_ASSERT_EQUAL(3, sec.relation_count);
    TEST_ASSERT_EQUAL(0, sec.relations[2].value.list->object_count);
}

static void test_file_with_multiple_sections(void) {
    stream = fopen("tests/test_file_with_multiple_sections.json", "r");

    Object actual = json_parse(stream);
    TEST_ASSERT_NO_ERROR();

    TEST_ASSERT_EQUAL(4, actual.relation_count);

    List *sections = actual.relations[3].value.list;
    TEST_ASSERT_EQUAL(3, sections->object_count);
    TEST_ASSERT_EQUAL(3, sections->elements[0].relation_count);
    TEST_ASSERT_EQUAL(3, sections->elements[1].relation_count);
    TEST_ASSERT_EQUAL(3, sections->elements[2].relation_count);

    compare_strings(
        (String){ .chars = "back to where we started", .len = 25 },
        sections->elements[2].relations[2].value.list->elements[0].relations[1].value.str
    );
}

// JSON to Adventure tests
static void test_convert_small_adventure(void) {
    stream = fopen("tests/test_file_with_single_section.json", "r");

    Adventure expected = (Adventure){
        .title = "first test file!",
        .author = "me",
        .version = "1.0",
        .section_count = 1,
        .sections = &(Section){
            .text = "This is the first and only section.",
            .id = 0,
            .option_count = 0,
            .options = NULL
        }
    };

    Adventure actual = json_to_adventure(json_parse(stream));

    TEST_ASSERT_NO_ERROR();
    compare_adventures(expected, actual);
}

static void test_convert_not_so_small_adventure(void) {
    stream = fopen("tests/test_file_with_multiple_sections.json", "r");

    Option o1 = (Option){
        .section_id = 1,
        .text = "to section A!",
    };
    Option o2 = (Option){
        .section_id = 2,
        .text = "to section B!",
    };
    Option o3 = (Option){
        .section_id = 0,
        .text = "back to where we started",
    };

    Option s0[] = {o1, o2};
    Option s1[] = {o3, o2};
    Option s2[] = {o3, o1};

    Section s[] = {
        (Section){
            .id = 0,
            .text = "This is the first and only section.",
            .option_count = 2,
            .options = s0
        },
        (Section){
            .id = 1,
            .text = "This is section A.",
            .option_count = 2,
            .options = s1
        },
        (Section){
            .id = 2,
            .text = "This is section B.",
            .option_count = 2,
            .options = s2
        },
    };
    Adventure expected = (Adventure){
        .title = "second test file!",
        .author = "me",
        .version = "1.0",
        .section_count = 3,
        .sections = s,
    };

    Adventure actual = json_to_adventure(json_parse(stream));

    TEST_ASSERT_NO_ERROR();
    compare_adventures(expected, actual);
}

static void test_convert_bigger_adventure(void) {
    stream = fopen("tests/test_file_bigger_adventure.json", "r");

    Adventure actual = json_to_adventure(json_parse(stream));

    TEST_ASSERT_NO_ERROR();
    TEST_ASSERT_EQUAL(5, actual.section_count);
    TEST_ASSERT_EQUAL_STRING(
        "this is the very first thing the user sees after running the game.",
        actual.sections[0].text
    );
    TEST_ASSERT_EQUAL(2, actual.sections[0].option_count);
    TEST_ASSERT_EQUAL_STRING(
        "oh, the user has selected this option! now the story has to continue somehow.",
        actual.sections[1].text
    );
    TEST_ASSERT_EQUAL(1, actual.sections[1].option_count);
    TEST_ASSERT_EQUAL_STRING(
        "So, after getting here, the story experiences some inconveniences. The plott thickens.",
        actual.sections[2].text
    );
    TEST_ASSERT_EQUAL(2, actual.sections[2].option_count);
    TEST_ASSERT_EQUAL_STRING(
        "The user changed the outcome and the main character is having a happy ending.",
        actual.sections[3].text
    );
    TEST_ASSERT_EQUAL(0, actual.sections[3].option_count);
    TEST_ASSERT_EQUAL_STRING(
        "The user didn't change the outcome and the main character dies.",
        actual.sections[4].text
    );
    TEST_ASSERT_EQUAL(0, actual.sections[4].option_count);
}

static void test_convert_adventure_missing_title(void) {
    stream = fopen("tests/test_file_missing_title.json", "r");

    Adventure actual = json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_missing_author(void) {
    stream = fopen("tests/test_file_missing_author.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_missing_version(void) {
    stream = fopen("tests/test_file_missing_version.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_missing_sections(void) {
    construct_file_like_obj("{\"title\":\"\",\"author\":\"\",\"version\":\"\"}");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_section_missing_id(void) {
    stream = fopen("tests/test_file_section_missing_id.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_section_missing_text(void) {
    stream = fopen("tests/test_file_section_missing_text.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_section_missing_options(void) {
    stream = fopen("tests/test_file_section_missing_options.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_option_missing_id(void) {
    stream = fopen("tests/test_file_option_missing_id.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

static void test_convert_adventure_option_missing_text(void) {
    stream = fopen("tests/test_file_option_missing_text.json", "r");

    json_to_adventure(json_parse(stream));

    TEST_ASSERT_ERROR(PE_MISSING_KEY);
}

int main() {
    UnityBegin("tests/text_adventure_tests.c");

    RUN_TEST(test_json_empty_str);
    RUN_TEST(test_jsom_empty_file_when_only_whitespace_present);
    RUN_TEST(test_json_empty_object);
    RUN_TEST(test_json_empty_mismatched_brackets);
    RUN_TEST(test_key_must_be_string);
    RUN_TEST(test_allow_string_with_whitespace_as_key);
    RUN_TEST(test_escaped_double_quote);
    RUN_TEST(test_escaped_unicode_chars);
    RUN_TEST(test_object_must_have_name_value_pair);
    RUN_TEST(test_object_must_have_name_value_pair2);
    RUN_TEST(test_invalid_double_string);
    RUN_TEST(test_invalid_double_quote);
    RUN_TEST(test_invalid_double_colon);
    RUN_TEST(test_invalid_double_colon2);
    RUN_TEST(test_incomplete_string);
    RUN_TEST(test_incomplete_object);
    RUN_TEST(test_relation_with_trailing_comma);
    RUN_TEST(test_relation_with_tow_trailing_commas);
    RUN_TEST(teset_object_with_two_relations);
    RUN_TEST(test_relation_with_numeric_value);
    RUN_TEST(test_invalid_relation_with_negative_signed_number);
    RUN_TEST(test_invalid_relation_with_positive_signed_number);
    RUN_TEST(test_invalid_relation_with_exponent);
    RUN_TEST(test_invalid_number_with_whitespace);
    RUN_TEST(test_number_surpases_limit);
    RUN_TEST(test_incomplete_number);
    RUN_TEST(test_multiple_numbered_relations);
    RUN_TEST(test_string_and_numbered_relations);
    RUN_TEST(test_empty_list);
    RUN_TEST(test_invalid_empty_list);
    RUN_TEST(test_invalid_empty_list_with_separator);
    RUN_TEST(test_list_with_incomplete_object);
    RUN_TEST(test_list_with_simple_object_inside);
    RUN_TEST(test_list_with_two_objects_inside);
    RUN_TEST(test_file_with_single_section);
    RUN_TEST(test_file_with_multiple_sections);
    RUN_TEST(test_convert_small_adventure);
    RUN_TEST(test_convert_not_so_small_adventure);
    RUN_TEST(test_convert_bigger_adventure);
    RUN_TEST(test_convert_adventure_missing_title);
    RUN_TEST(test_convert_adventure_missing_author);
    RUN_TEST(test_convert_adventure_missing_version);
    RUN_TEST(test_convert_adventure_missing_sections);
    RUN_TEST(test_convert_adventure_section_missing_id);
    RUN_TEST(test_convert_adventure_section_missing_text);
    RUN_TEST(test_convert_adventure_section_missing_options);
    RUN_TEST(test_convert_adventure_option_missing_id);
    RUN_TEST(test_convert_adventure_option_missing_text);

    return UnityEnd();
}
