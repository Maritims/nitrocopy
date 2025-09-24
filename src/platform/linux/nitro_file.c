#include "nitro_file.h"

#include <dirent.h>
#include <unistd.h>

DIR* nitro_file_opendir(const char* path) {
    return opendir(path);
}

struct dirent* nitro_file_readdir(DIR* dir) {
    if(!dir) {
        return NULL;
    }

    return readdir(dir);
}

int nitro_file_closedir(DIR* dir) {
    if(!dir) {
       return -1;
    }

    return closedir(dir);
}

int nitro_file_mkdir(const char* path, int mode) {
    return mkdir(path, mode);
}

int nitro_file_stat(const char* path, struct stat* file_stats) {
    if(stat(path, file_stats) != 0) {
        return -1;
    }
    
    return 0;
}
