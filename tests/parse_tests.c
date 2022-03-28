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
#define TEST_ASSERT_STATE(ps)    TEST_ASSERT_EQUAL(ps, parse_state)
#define TEST_ASSERT_ERROR(pe)    TEST_ASSERT_EQUAL(pe, parse_error)
#define TEST_ASSERT_NO_ERROR()    TEST_ASSERT_STATE(PS_OK)

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

// JSON to Adventure conversion tests
// ...

int main() {
    UnityBegin("tests/parse_tests.c");

    RUN_TEST(test_json_empty_str);
    RUN_TEST(test_jsom_empty_file_when_only_whitespace_present);
    RUN_TEST(test_json_empty_object);

    return UnityEnd();
}
