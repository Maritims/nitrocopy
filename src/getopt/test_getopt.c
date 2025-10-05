#include "getopt.h"
#include <stdio.h>
#include <string.h>

typedef int (*test_fn)(void);

struct test_case_t {
    const char* name;
    char        expected;
    test_fn     fn;
};

int test_option_with_attached_argument(void) {
    int argc        = 2;
    char* argv[2]   = {"foobar", "-ar"};
    char* optstring = "a:";
    int actual      = getopt(argc, argv, optstring);

    if(actual != 'a') {
        fprintf(stderr, "expected 'a', but got %c\n", (char) actual);
        return -1;
    }

    if(optarg == NULL) {
        fprintf(stderr, "expected optarg to not be NULL, but it was NULL\n");
        return -1;
    }

    if(optarg[0] != 'r') {
        fprintf(stderr, "expected optarg[0] to be 'r', but it was %c\n", (char) optarg[0]);
        return -1;
    }

    return 'a';
}

int test_option_with_detached_argument(void) {
    int argc        = 3;
    char* argv[3]   = {"foobar", "-a", "r"};
    char* optstring = "a:";
    int actual      = getopt(argc, argv, optstring);

    if(actual != 'a') {
        fprintf(stderr, "expected 'a', but got %c\n", (char) actual);
        return actual;
    }

    if(optarg == NULL) {
        fprintf(stderr, "expected optarg to not be NULL, but it was NULL\n");
        return -1;
    }

    if(optarg[0] != 'r') {
        fprintf(stderr, "expected optarg[0] to be 'r', but it was %c\n", (char) optarg[0]);
        return -1;
    }

    return actual;
}

int test_option_without_argument(void) {
    int argc = 2;
    char* argv[2] = {"foobar", "-a"};
    char* optstring = "a";
    int actual = getopt(argc, argv, optstring);

    if(actual != 'a') {
        fprintf(stderr, "expected 'a', but got %c\n", (char) actual);
        return actual;
    }

    if(optarg != NULL) {
        fprintf(stderr, "expected optarg to be NULL, but it was %s\n", optarg);
        return -1;
    }

    return actual;
}

int test_option_and_non_options(void) {
    int argc = 5;
    char* argv[5] = {"foobar", "foo", "-a", "bar", "-h"};
    char* optstring = "ah";
    int actual = getopt(argc, argv, optstring);

    if(actual != 'a') {
        fprintf(stderr, "expected 'a', but got %c\n", (char) actual);
        return actual;
    }

    if(optarg != NULL) {
        fprintf(stderr, "expected optarg to be NULL, but it was %s\n", optarg);
        return -1;
    }

    if(strcmp(argv[1], "-a") != 0) {
        fprintf(stderr, "expected '-a' at argv[1], but it was %s\n", argv[1]);
        return -1;
    }

    if(strcmp(argv[2], "-h") != 0) {
        fprintf(stderr, "expected '-h' at argv[2], but it was %s\n", argv[2]);
        return -1;
    }

    if(strcmp(argv[3], "foo") != 0) {
        fprintf(stderr, "expected 'foo' at argv[3], but it was %s\n", argv[3]);
        return -1;
    }

    if(strcmp(argv[4], "bar") != 0) {
        fprintf(stderr, "expected 'bar' at argv[4], but it was %s\n", argv[4]);
        return -1;
    }

    return actual;
}

int main(void) {
    struct test_case_t test_cases[] = {
        {"option with attached argument", 'a', test_option_with_attached_argument},
        {"option with detached argument", 'a', test_option_with_detached_argument},
        {"option without argument", 'a', test_option_without_argument},
        {"option and non-options", 'a', test_option_and_non_options}
    };
    int i, len, exit_code;

    len = sizeof(test_cases) / sizeof(test_cases[0]);
    exit_code = 0;

    for(i = 0; i < len; i++) {
        int actual;

        /* Reset optind, optopt and optarg between each test execution */
        optind = 1;
        optopt = -1;
        optarg = NULL;

        actual = test_cases[i].fn();

        if(test_cases[i].expected != actual) {
            fprintf(stderr, "Test \"%s\" failed. Expected %d, but got %d\n", test_cases[i].name, test_cases[i].expected, actual);
            exit_code = 1;
        }
    }

    return exit_code;
}
