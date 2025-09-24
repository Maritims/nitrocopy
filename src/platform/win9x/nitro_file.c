#include "nitro_file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <io.h>
#include <sys/stat.h>

DIR* nitro_file_opendir(const char* path) {
    DIR* dir;
    char search_path[MAX_PATH];
    
    dir = (DIR*)malloc(sizeof(DIR));
    if(!dir) {
        return NULL;
    }

    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    dir->handle = FindFirstFile(search_path, &(dir->find_data));
    dir->is_first_entry = 1;
    if(dir->handle == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    return dir;
}

struct dirent* nitro_file_readdir(DIR* dir) {
    if(!dir) {
        return NULL;
    }

    while (dir->is_first_entry || FindNextFile(dir->handle, &(dir->find_data))) {
        dir->is_first_entry = 0;

        if(strcmp(dir->find_data.cFileName, ".") == 0 || strcmp(dir->find_data.cFileName, "..") == 0) {
            continue;
        }
        
        strncpy(dir->entry.d_name, dir->find_data.cFileName, sizeof(dir->entry.d_name) - 1);
        dir->entry.d_name[sizeof(dir->entry.d_name) - 1] = '\0';
        return &(dir->entry);
    }

    return NULL;
}

int nitro_file_closedir(DIR* dir) {
    if(!dir) {
        return -1;
    }

    FindClose(dir->handle);
    free(dir);
    return 0;
}

int nitro_file_mkdir(const char* path, int mode) {
    return mkdir(path);
}

int nitro_file_stat(const char* path, struct stat* file_stats) {
    /* Make sure we use the correct struct type. */
    struct _stat64i32 win_stats;
    if(_stat64i32(path, &win_stats) != 0) {
        return 1;
    }

    /* Manually copy the relevant fields. */
    file_stats->st_mode = win_stats.st_mode;
    file_stats->st_size = win_stats.st_size;

    return 0;
}
