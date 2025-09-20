#include "compat.h"

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

DIR* compat_opendir(const char* path) {
#if defined(_WIN32)
    DIR* dir = (DIR*)malloc(sizeof(DIR));
    if(!dir) {
        return NULL;
    }
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);
    dir->handle = FindFirstFile(search_path, &(dir->find_data));
    dir->is_first_entry = 1;
    if(dir->handle == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    return dir;
#else
    return opendir(path);
#endif
}

struct dirent* compat_readdir(DIR* dir) {
    if(!dir) {
        return NULL;
    }

#if defined(_WIN32)
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
#else
    return readdir(dir);
#endif
}

int compat_closedir(DIR* dir) {
    if(!dir) {
       return -1;
    }
#if defined(_WIN32)
    FindClose(dir->handle);
    free(dir);
    return 0;
#else
    return closedir(dir);
#endif
}

int compat_mkdir(const char* path, int mode) {
#if defined(_WIN32)
    return mkdir(path);
#else
    return mkdir(path, mode);
#endif
}
