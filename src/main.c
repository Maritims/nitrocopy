#include "copier.h"
#include "getopt.h"
#include "nitro_version.h"
#include "runtime.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

int overwrite                                   = 0;
copy_existing_action_t g_default_copy_action    = COPY_ACTION_UNSET;

int main(int argc, char *argv[]) {
    int opt;
    char* src_path  = NULL;
    char* dest_path = NULL;

    while ((opt = getopt(argc, argv, "a:vh")) != -1) {
        switch(opt) {
            case 'a':
                if(optarg[0] == 'r') {
                    g_default_copy_action = COPY_RESUME;
                } else if(optarg[0] == 's') {
                    g_default_copy_action = COPY_SKIP;
                } else if(optarg[0] == 'o') {
                    g_default_copy_action = COPY_OVERWRITE;
                } else {
                    fprintf(stderr, "Invalid argument for -a: %s\n", optarg);
                    return 1;
                }
            break;
            case 'v':
                printf("NitroCopy version: %s\nhttps://github.com/maritims/nitrocopy\n\nWritten by Martin Severin Steffensen.\n", NITRO_VERSION_STRING);
                return 0;
            case 'h':
                printf("NitroCopy %s - A file copy utility for some modern systems and some not so modern systems.\n\n", NITRO_VERSION_STRING);
                printf("Usage: NitroCopy [OPTIONS] <source_path> <destination_path>\n");
                printf("Options:\n");
                printf("    -a, --action        Default action to take when a destination file exists: [r]esume, [s]kip or [o]verwrite.\n");
                printf("    -v, --version       Show version.\n");
                printf("    -h, --help          Display this help: message and exit.\n");

                printf("\nFor more information:\n");
                printf("    Documentation:  https://github.com/maritims/nitrocopy#readme\n");
                printf("    Bug reports:    https://github.com/maritims/nitrocopy/issues\n");
                printf("    General info:   https://github.com/maritims\n");

                printf("\nWritten by Martin Severin Steffensen.\n");
                return 0;
            break;
            case '?':
                return 1;
        }
    }

    if(optind < argc) {
        src_path = argv[optind++];
    }
    if(optind < argc) {
        dest_path = argv[optind++];
    }
    if(!src_path || !dest_path) {
        fprintf(stderr, "Error: Missing source or destination path.\n");
        fprintf(stderr, "Use %s --help for usage information.\n", argv[0]);
        return 1;
    }

    printf("NitroCopy %s - (overwrite: %d)\n\n", NITRO_VERSION_STRING, overwrite);

    return copier_execute(src_path, dest_path);
}
