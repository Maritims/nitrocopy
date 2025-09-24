#ifndef NITRO_FILE_COMPAT_H
#define NITRO_FILE_COMPAT_H

#include <stdio.h>
#include <sys/stat.h>


#if defined(_WIN32)
#include <windows.h>

struct dirent {
    char d_name[MAX_PATH];
};

typedef struct {
    HANDLE handle;
    WIN32_FIND_DATA find_data;
    int is_first_entry;
    struct dirent entry;
} DIR;
#else
#include <dirent.h>
#endif

DIR* nitro_file_opendir(const char* path);

struct dirent* nitro_file_readdir(DIR* dir);

int nitro_file_closedir(DIR* dir);

int nitro_file_mkdir(const char* path, int mode);

int nitro_file_stat(const char* path, struct stat* file_stats);

#endif
