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

static void construct_file_like_obj(char *s) {
    size_t ssize = strlen(s) + 1;
    buffer = malloc(sizeof(char) * ssize);
    stream = fmemopen(buffer, ssize, "w+");
    fprintf(stream, "%s", s);
    rewind(stream);
}


// JSON parse tests

static void test_json_empty_str(void) {
    construct_file_like_obj("");

    json_parse(stream);
    TEST_ASSERT_ERROR(PE_EMPTY_FILE);
}

// JSON to Adventure tests

int main() {
    UnityBegin("tests/parse_tests.c");

    RUN_TEST(test_json_empty_str);

    return UnityEnd();
}
