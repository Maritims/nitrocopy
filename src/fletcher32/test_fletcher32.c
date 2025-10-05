#include "fletcher32.h"
#include "word16.h"
#include <string.h>

struct test_case_t {
    const char*     name;
    const char*     input;
    unsigned long   expected;
};

int main(void) {
    struct test_case_t test_cases[] = {
        {"empty input", NULL, 0xFFFFFFFF},
        {"abcde", "abcde", 0xF04FC729},
        {"abcdef", "abcdef", 0x56502D2A},
        {"abcdefgh", "abcdefgh", 0xEBE19591}
    };
    int i, len, exit_code;

    len = sizeof(test_cases) / sizeof(test_cases[0]);
    exit_code = 0;

    for(i = 0; i < len; i++) {
        size_t          number_of_words;
        unsigned short  data[10];
        unsigned long   actual;

        if(test_cases[i].input == NULL) {
            actual = fletcher32(NULL, 0);
        } else {
            number_of_words = word16_count(strlen(test_cases[i].input));
            word16_create((const unsigned char*) test_cases[i].input, test_cases[i].input == NULL ? 0 : strlen(test_cases[i].input), data);
            actual = fletcher32(data, number_of_words);
        }

        if(test_cases[i].expected != actual) {
            fprintf(stderr, "Test \"%s\" failed. Expected %lu, but got %lu\n", test_cases[i].name, test_cases[i].expected, actual);
            exit_code = 1;
        }
    }

    return exit_code;
}
