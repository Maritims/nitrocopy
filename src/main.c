#include "libnitro.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    struct stat file_stats;
    if (stat(argv[1], &file_stats) == -1) {
        fprintf(stderr, "Error: Could not get source path info for '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    NitroCopyState* state = nitro_init();
    if(state == NULL) {
        return 1;
    }

    nitro_copy(state, argv[1], argv[2]);


    return 0;
}
