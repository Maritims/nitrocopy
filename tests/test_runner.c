#include "test_runner.h"
#include <stdio.h>

int assert_long_equals(long expected, long actual, char* out_message_buffer) {
    if(expected == actual) {
        return 1;
    }

    sprintf(out_message_buffer, "expected: %lu, actual: %lu", expected, actual);
    return 0;
}

int run_tests(const struct test_case* tests, int count) {
    int i;
    int passed = 0;
    int failed = 0;

    printf("Running %d tests...\n", count);

    for(i = 0; i < count; i++) {
        char out_message_buffer[256];
        int success = tests[i].fn(out_message_buffer);
        if(success) {
            printf("[PASS] %s\n", tests[i].name);
            passed++;
        } else {
            printf("[FAIL] %s (%s)\n", tests[i].name, out_message_buffer);
            failed++;
        }
    }

    printf("%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
