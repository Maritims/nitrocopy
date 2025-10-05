#ifndef COPIER_H
#define COPIER_H

#include <stdio.h>

typedef enum {
    COPY_RESUME,
    COPY_OVERWRITE,
    COPY_SKIP
} copy_existing_action_t;

extern copy_existing_action_t g_default_copy_action;

#define COPY_ACTION_UNSET ((copy_existing_action_t)-1)

typedef void (*ProgressCallback)(void);

int copier_copy(const char* src, const char* dest, size_t total_files, size_t total_bytes, size_t* files_copied, size_t* bytes_copied);

size_t copier_copy_file(const char* src, const char* dest);

int copier_copy_dir(const char* src_dir, const char* dest_dir, size_t* total_files, size_t* total_bytes);

int copier_execute(const char* src, const char* dest);

int copier_get_total_stats(const char* path, size_t* total_files, size_t* total_bytes);

#endif
