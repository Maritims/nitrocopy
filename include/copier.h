#ifndef COPIER_H
#define COPIER_H

#include "copy_process.h"
 
int copier_copy(const char* src, const char* dest, copy_process_t* state);

int copier_copy_file(const char* src, const char* dest, copy_process_t* state);

int copier_copy_dir(const char* src_dir, const char* dest_dir, copy_process_t* state);

int copier_get_total_stats(const char* path, copy_process_t* state);

#endif
