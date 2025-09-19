#include "libnitro.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    int overwrite = 0;
    char* src_path = NULL;
    char* dest_path = NULL;

    static struct option long_options[] = {
        {"overwrite", no_argument, 0, 'o'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "oh", long_options, NULL)) != -1) {
        switch(opt) {
            case 'o':
                overwrite = 1;
            break;
            case 'h':
                printf("Usage: %s [OPTIONS] <source_path> <destination_path>\n", argv[0]);
                printf("Options:\n");
                printf("    -o, --overwrite     Overwrite destination files without prompting.\n");
                printf("    -h, --help          Display this help message and exit.\n");
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

    NitroCopyState* state = nitro_init(overwrite);
    if(state == NULL) {
        return 1;
    }

    NitroStatus status = nitro_copy(state, src_path, dest_path);
    return status;
}
