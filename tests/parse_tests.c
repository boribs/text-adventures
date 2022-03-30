#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "unity/unity.h"
#include "../src/common.h"
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
    TEST_FAIL_MESSAGE("Not implemented.");
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

static void test_key_must_be_string(void) {
    construct_file_like_obj("{key:\"value\"}");

    Object actual = json_parse(stream);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 2);
}

static void test_allow_string_with_whitespace_as_key(void) {
    construct_file_like_obj("{\"key with whitespace\":\"val\"}");

    Object actual = json_parse(stream);
    Object expected = (Object){
        .relation_count = 1,
        .relations = (Relation *){
            &(Relation){
                .key = (String){
                    .chars = "key with whitespace",
                    .len = 20
                },
                .value_type = VALUE_STR,
                .value = (Value){.str = (String){
                    .chars = "val",
                    .len = 4
                }}
            }
        }
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

static void test_invalid_double_colon(void) {
    construct_file_like_obj("{\"valid\\\"key\"::\"valid value\"}");

    json_parse(stream);
    TEST_ASSERT_STATE(PS_ERROR);
    TEST_ASSERT_ERROR(PE_INVALID_CHAR);
    TEST_ASSERT_POSITION(0, 15);
}

// JSON to Adventure conversion tests
// ...

int main() {
    UnityBegin("tests/parse_tests.c");

    RUN_TEST(test_json_empty_str);
    RUN_TEST(test_jsom_empty_file_when_only_whitespace_present);
    RUN_TEST(test_json_empty_object);
    RUN_TEST(test_key_must_be_string);
    RUN_TEST(test_allow_string_with_whitespace_as_key);
    RUN_TEST(test_escaped_double_quote);
    RUN_TEST(test_escaped_unicode_chars);
    RUN_TEST(test_object_must_have_name_value_pair);
    RUN_TEST(test_object_must_have_name_value_pair2);
    RUN_TEST(test_invalid_double_string);
    RUN_TEST(test_invalid_double_colon);

    return UnityEnd();
}
