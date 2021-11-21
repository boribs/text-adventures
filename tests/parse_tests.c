#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "unity/unity.h"
#include "../src/common.h"
#include "../src/parse.h"

FILE *stream;
char *buffer;
struct Adventure a;
struct TokenError e;

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

#define TEST_ASSERT_EQUAL_E_STATE(s)    TEST_ASSERT_EQUAL(s, e.state);

static void construct_file_like_obj(char *s) {
    size_t ssize = strlen(s) + 1;
    buffer = malloc(sizeof(char) * ssize);
    stream = fmemopen(buffer, ssize, "w+");
    fprintf(stream, "%s", s);
    rewind(stream);
}

static void parse_very_simple_and_short_str() {
    char s[] = "a\nb\nc\n <0> aksdjfask[<1> opcion 1] <1> aksdjfask[<0> opcion 2]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_OK);
    TEST_ASSERT_EQUAL_STRING("a", a.title);
    TEST_ASSERT_EQUAL_STRING("b", a.author);
    TEST_ASSERT_EQUAL_STRING("c", a.version);

    TEST_ASSERT_EQUAL(2, a.sec_count);

    TEST_ASSERT_EQUAL(0, a.sections[0].id);
    TEST_ASSERT_EQUAL_STRING("aksdjfask", a.sections[0].text);
    TEST_ASSERT_EQUAL(1, a.sections[0].opt_count);
    TEST_ASSERT_EQUAL_STRING("opcion 1", a.sections[0].options[0].text);

    TEST_ASSERT_EQUAL(1, a.sections[1].id);
    TEST_ASSERT_EQUAL_STRING("aksdjfask", a.sections[1].text);
    TEST_ASSERT_EQUAL(0, a.sections[1].options[0].sec_id);
    TEST_ASSERT_EQUAL_STRING("opcion 2", a.sections[1].options[0].text);
}

static void parse_correct_syntax_file_with_single_section() { // expect error
    char s[] = "    Adventure1 \nCristian Gotchev\nv0\n<0> Once upon a time\n[<0> Twice upon a time]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_VERY_FEW_SECTIONS_IN_ADVENTURE);
}

static void parse_text_file_1() {
    stream = fopen("tests/t1.txt", "r");
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_OK);
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

static void parse_adventure_with_incorrect_id_syntax() {
    char s[] = "a\na\na\n<1a>";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_CHAR_IN_ID);
}

static void parse_adventure_with_incorrect_id_syntax_2() {
    char s[] = "a\na\na\n<1>s[<njn> asdfas]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_CHAR_IN_ID);
}

static void parse_adventure_with_incorrect_option_syntax() {
    char s[] = "a\na\na\n<1>asfd[<1>]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_SYNTAX_EXPECTED_TEXT);
}

static void parse_adventure_with_incorrect_option_syntax_2() {
    char s[] = "a\na\na\n<1>asfd[asfasd]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_SYNTAX_EXPECTED_ID);
}

static void parse_adventure_with_incorrect_section_syntax() {
    char s[] = "a\na\na\n<1>[<1> asdfas]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_SYNTAX_EXPECTED_TEXT);
}

static void parse_adventure_with_incorrect_section_syntax_2() {
    char s[] = "a\na\na\nasdkjf[<1> asdfas]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_INVALID_SYNTAX_EXPECTED_ID);
}

static void parse_adventure_empty_file() {
    char s[] = "";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_NOT_EQUAL(P_STATE_OK, e.state);
}

static void parse_error_too_many_options_in_section() {
    char s[] = "a\na\na\n<0>asdkjf[<1>a<1>a<1>a<1>a<1>a<1>a<1>a]";
    construct_file_like_obj(S);
    e = parse(stream, &a);

    TEST_ASSERT_EQUAL_E_STATE(P_STATE_TOO_MANY_OPTIONS_IN_SECTION);
}

int main() {
    UnityBegin("tests/parse_tests.c");

    RUN_TEST(parse_very_simple_and_short_str);
    RUN_TEST(parse_correct_syntax_file_with_single_section);
    RUN_TEST(parse_text_file_1);
    RUN_TEST(parse_adventure_with_incorrect_id_syntax);
    RUN_TEST(parse_adventure_with_incorrect_id_syntax_2);
    RUN_TEST(parse_adventure_with_incorrect_option_syntax);
    RUN_TEST(parse_adventure_with_incorrect_option_syntax_2);
    RUN_TEST(parse_adventure_with_incorrect_section_syntax);
    RUN_TEST(parse_adventure_with_incorrect_section_syntax_2);
    RUN_TEST(parse_adventure_empty_file);
    RUN_TEST(parse_error_too_many_options_in_section);

    return UnityEnd();
}
