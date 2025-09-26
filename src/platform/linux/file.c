#include "file.h"

#include <dirent.h>
#include <unistd.h>

DIR* compat_opendir(const char* path) {
    return opendir(path);
}

struct dirent* compat_readdir(DIR* dir) {
    if(!dir) {
        return NULL;
    }

    return readdir(dir);
}

int compat_closedir(DIR* dir) {
    if(!dir) {
       return -1;
    }

    return closedir(dir);
}

int compat_mkdir(const char* path, int mode) {
    return mkdir(path, mode);
}

int compat_stat(const char* path, struct stat* file_stats) {
    if(stat(path, file_stats) != 0) {
        return -1;
    }
    
    return 0;
}
