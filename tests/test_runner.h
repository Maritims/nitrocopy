#ifndef TEST_RUNNER
#define TEST_RUNNER

typedef int (*test_fn)(char*);

struct test_case {
    const char* name;
    test_fn     fn;
};

int assert_long_equals(long expected, long actual, char* out_message_buffer);

int run_tests(const struct test_case* tests, int count);

#endif
