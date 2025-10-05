#ifndef GETOPT_H
#define GETOPT_H

extern char*    optarg;
extern int      optind;
extern int      opterr;
extern int      optopt;

int swap_with_tail(int argc, char* argv[], int n);

int getopt(int argc, char *argv[], const char* optstring);

#endif
