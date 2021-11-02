#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "unity/unity.h"
#include "../src/common.h"
#include "../src/parse.h"

FILE *stream;
char *buffer;
struct Adventure a;

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

    // clear adventure
}

#define S &s[0]

static void construct_file_like_obj(char *s) {
    size_t ssize = strlen(s) + 1;
    buffer = malloc(sizeof(char) * ssize);
    stream = fmemopen(buffer, ssize, "w+");
    fprintf(stream, "%s", s);
    rewind(stream);
}

static void parse_correct_syntax_file_with_single_section() {
    char s[] = "    Adventure1 \nCristian Gotchev\nv0\n1\n<0> Once upon a time\n[<0> Twice upon a time]";
    construct_file_like_obj(S);

    TEST_ASSERT_TRUE(parse(stream, &a));
    TEST_ASSERT_EQUAL_STRING("Adventure1", a.title);
    TEST_ASSERT_EQUAL_STRING("Cristian Gotchev", a.author);
    TEST_ASSERT_EQUAL_STRING("v0", a.version);
    TEST_ASSERT_EQUAL_STRING("Once upon a time", a.sections[0].text);
    TEST_ASSERT_EQUAL(1, a.sections[0].opt_count);
    TEST_ASSERT_EQUAL(0, a.sections[0].options[0].sec_id);
    TEST_ASSERT_EQUAL_STRING("Twice upon a time", a.sections[0].options[0].text);
}

static void parse_correct_syntax_file_with_two_sections() {
    char s[] = "A\nB\nv0\n2\n\n<0> Section 0\n[ <1> option to 1\n <1> another option to 1]";
    construct_file_like_obj(S);

    TEST_ASSERT_TRUE(parse(stream, &a));
    TEST_ASSERT_EQUAL_STRING("A", a.title);
    TEST_ASSERT_EQUAL_STRING("B", a.author);
    TEST_ASSERT_EQUAL_STRING("v0", a.version);
    TEST_ASSERT_EQUAL_STRING("Section 0", a.sections[0].text);
    TEST_ASSERT_EQUAL(2, a.sections[0].opt_count);
    TEST_ASSERT_EQUAL(1, a.sections[0].options[0].sec_id);
    TEST_ASSERT_EQUAL(1, a.sections[0].options[1].sec_id);
    TEST_ASSERT_EQUAL_STRING("option to 1", a.sections[0].options[0].text);
    TEST_ASSERT_EQUAL_STRING("another option to 1", a.sections[0].options[1].text);
}

static void parse_text_file_1() {
    stream = fopen("tests/t1.txt", "r");

    TEST_ASSERT_TRUE(parse(stream, &a));
    TEST_ASSERT_EQUAL_STRING("Adventure of a lifetime", a.title);
    TEST_ASSERT_EQUAL_STRING("Cristian Gotchev", a.author);
    TEST_ASSERT_EQUAL_STRING("v1", a.version);

    TEST_ASSERT_EQUAL_STRING("Section 0\nhas more lines in its text", a.sections[0].text);
    TEST_ASSERT_EQUAL(2, a.sections[0].opt_count);
    TEST_ASSERT_EQUAL_STRING("to option 1", a.sections[0].options[0].text);
    TEST_ASSERT_EQUAL(1, a.sections[0].options[0].sec_id);
    TEST_ASSERT_EQUAL_STRING("to option 2", a.sections[0].options[1].text);
    TEST_ASSERT_EQUAL(2, a.sections[0].options[1].sec_id);

    TEST_ASSERT_EQUAL_STRING("Section 1", a.sections[1].text);
    TEST_ASSERT_EQUAL(2, a.sections[1].opt_count);
    TEST_ASSERT_EQUAL_STRING("to option 2", a.sections[1].options[0].text);
    TEST_ASSERT_EQUAL(2, a.sections[1].options[0].sec_id);
    TEST_ASSERT_EQUAL_STRING("to option 0", a.sections[1].options[1].text);
    TEST_ASSERT_EQUAL(0, a.sections[1].options[1].sec_id);

    TEST_ASSERT_EQUAL_STRING("Section 2", a.sections[2].text);
    TEST_ASSERT_EQUAL(1, a.sections[2].opt_count);
    TEST_ASSERT_EQUAL_STRING("to option 1", a.sections[2].options[0].text);
    TEST_ASSERT_EQUAL(1, a.sections[2].options[0].sec_id);
}

static void parse_text_file_2_char_instead_of_number_on_4th_line() {
    stream = fopen("tests/t2.txt", "r");

    TEST_ASSERT_FALSE(parse(stream, &a));
}

static void parse_text_file_3_with_incorrect_syntax() {
    stream = fopen("tests/t3.txt", "r");

    TEST_ASSERT_FALSE(parse(stream, &a));
}

int main() {
    UnityBegin("tests/parse_tests.c");

    RUN_TEST(parse_correct_syntax_file_with_single_section);
    RUN_TEST(parse_correct_syntax_file_with_two_sections);
    RUN_TEST(parse_text_file_1);
    RUN_TEST(parse_text_file_2_char_instead_of_number_on_4th_line);
    RUN_TEST(parse_text_file_3_with_incorrect_syntax);

    return UnityEnd();
}
