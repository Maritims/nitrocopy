#include "fletcher32.h"
#include "test_runner.h"
#include "word16.h"
#include <stdlib.h>
#include <string.h>


int test_empty_input(char* out_message_buffer) {
    return assert_long_equals(0xFFFFFFFF, fletcher32(NULL, 0), out_message_buffer);
}

int test_abcde(char* out_message_buffer) {
    int             result;
    size_t          number_of_words;
    unsigned short* data;

    number_of_words = word16_count("abcde");
    data            = (unsigned short*) malloc(sizeof(short) * number_of_words);
    word16_create("abcde", data);

    result = assert_long_equals(0xF04FC729, fletcher32(data, number_of_words), out_message_buffer);

    free(data);
    return result;
}

int test_abcdef(char* out_message_buffer) {
    int result;
    size_t number_of_words;
    unsigned short* data;

    number_of_words = word16_count("abcdef");
    data = (unsigned short*) malloc(sizeof(short) * number_of_words);
    word16_create("abcdef", data);

    result = assert_long_equals(0x56502D2A, fletcher32(data, number_of_words), out_message_buffer);

    free(data);
    return result;
}

int test_abcdefgh(char* out_message_buffer) {
    int result;
    size_t number_of_words;
    unsigned short* data;

    number_of_words = word16_count("abcdefgh");
    data = (unsigned short*) malloc(sizeof(short) * number_of_words);
    word16_create("abcdefgh", data);

    result = assert_long_equals(0xEBE19591, fletcher32(data, number_of_words), out_message_buffer);

    free(data);
    return result;
}

static struct test_case tests[] = {
    {"empty input", test_empty_input},
    {"abcde", test_abcde},
    {"abcdef", test_abcdef},
    {"abcdefgh", test_abcdefgh}
};

int main(void) {
    return run_tests(tests, sizeof(tests) / sizeof(tests[0]));
}
