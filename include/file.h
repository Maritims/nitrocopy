#ifndef FILE_COMPAT_H
#define FILE_COMPAT_H

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

DIR* compat_opendir(const char* path);

struct dirent* compat_readdir(DIR* dir);

int compat_closedir(DIR* dir);

int compat_mkdir(const char* path, int mode);

int compat_stat(const char* path, struct stat* file_stats);

#endif
