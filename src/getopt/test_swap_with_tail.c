#include "getopt.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    char*   argv[3];
    int     argc = 3;
    int     n = 1;

    argv[0] = "foo";
    argv[1] = "bar";
    argv[2] = "baz";

    if(swap_with_tail(argc, argv, n) != 0) {
        fprintf(stderr, "swap_with_tail returned non-zero exit code\n");
        return 1;
    }

    if(strcmp(argv[argc - 1], "bar") != 0) {
        fprintf(stderr, "Unexpected value at tail position: %s\n", argv[argc - 1]);
        return -1;
    }

    if(strcmp(argv[n], "baz") != 0) {
        fprintf(stderr, "Unexpected value at n position: %s\n", argv[n]);
        return -1;
    }

    return 0;
}
