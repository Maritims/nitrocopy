#include "copier.h"
#include "nitro_version.h"
#include "copy_process.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    int overwrite           = 0;
    char* src_path          = NULL;
    char* dest_path         = NULL;
    copy_process_t* state   = NULL;

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
            case 'v':
                printf("NitroCopy version: %s\nhttps://github.com/maritims/nitrocopy\n\nWritten by Martin Severin Steffensen.\n", NITRO_VERSION_STRING);
                return 0;
            case 'h':
                printf("NitroCopy %s - A file copy utility for some modern systems and some not so modern systems.\n\n", NITRO_VERSION_STRING);
                printf("Usage: NitroCopy [OPTIONS] <source_path> <destination_path>\n");
                printf("Options:\n");
                printf("    -o, --overwrite     Overwrite destination files without prompting.\n");
                printf("    -v, --version       Show version.\n");
                printf("    -h, --help          Display this help message and exit.\n");

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

    state = copy_process_init(overwrite);
    if(state == NULL) {
        return 1;
    }

    return copier_copy(src_path, dest_path, state);
}
