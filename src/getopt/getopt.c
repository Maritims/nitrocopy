#include <stddef.h>
#include <stdio.h>
#include "getopt.h"

int     optind = 1; /* Which argv we're looking at. */
char*   optarg;     /* Argument for an option.      */
int     opterr = 1; /* Nonzero means print errors.  */
int     optopt;     /* Current option char.         */

static char* nextchar;

#define SWAP_ERROR(msg) \
    do { if (opterr) fprintf(stderr, "Error: %s\n", msg); return 1; } while(0)

int swap_with_tail(int argc, char* argv[], int n) {
    char* non_option;
    char* tail;

    if(!argv)       SWAP_ERROR("argv is NULL");
    if(argc <= 0)   SWAP_ERROR("argc must be greater than zero");
    if(n < 0)       SWAP_ERROR("n must be greater than or equal to zero");
    if(n >= argc)   SWAP_ERROR("n must be less than argc");

    non_option  = argv[n];
    tail        = argv[argc - 1];

    argv[argc - 1]  = non_option;
    argv[n]         = tail;

    return 0;
}

int getopt(int argc, char* argv[], const char* optstring) {
    const char* p;

    if(optstring == NULL) {
        if(opterr) {
            fprintf(stderr, "Error: optstring is required\n");
        }
        return -1;
    }

    if(optind >= argc) {
        /* No more arguments */
        return -2;
    }

    /* Are we looking at an option? */
    if(argv[optind][0] != '-' || argv[optind][1] == '\0') {
        /* By default, getopt() permutes the contents of argv as it scans, so that eventually all the nonoptions are at the end. */
        if(swap_with_tail(argc, argv, optind) != 0) {
            return -3;
        }

        argc--;
    
        return getopt(argc, argv, optstring);
    }

    /* Are we looking at the special argument "--"? */
    if(argv[optind][1] == '-' && argv[optind][2] == '\0') {
        /* The special argument "--" forces an end of option-scanning regardless of the scanning mode. */
        optind++;
        return -4;
    }

    if(nextchar == NULL) {
        /* The character to start with in the current argv */
        nextchar = argv[optind++] + 1;
    }

    /* Pull the next char and check if it's valid. */
    optopt = *nextchar++;

    /* Point into optstring for iteration. */
    p = optstring;

    while(*p != '\0') {
        if(*p == optopt) {
            /* The char is valid. Check if an argument is required. */
            if(p[1] == ':') {
                if(*nextchar != '\0') {
                    /* The argument is attached, example: -Dfoo */
                    optarg      = nextchar;
                    nextchar    = NULL;
                } else if(optind < argc) {
                    /* The argument is in the next argv, example: -D foo */
                    optarg      = argv[optind++];
                    nextchar    = NULL;
                } else {
                    if(opterr) {
                        fprintf(stderr, "Error: Option \"%c\" requires an argument.\n", optopt);
                    }

                    return ':';
                }
            }

            if(nextchar != NULL && *nextchar == '\0') {
                nextchar = NULL;
            }

            return optopt;
        }
        p++;
    }

    /* Return a question mark if the option char is unknown. */
    if(opterr) {
        fprintf(stderr, "Error: \"%c\" is an unknown option.\n", (char) optopt);
    }
    return '?';
}
