#ifndef COPY_PROCESS_H
#define COPY_PROCESS_H

#include <stdio.h>

typedef struct copy_process_t copy_process_t;

/* Getters */
size_t copy_process_get_total_size(const copy_process_t* state);

size_t copy_process_get_total_files(const copy_process_t* state);

size_t copy_process_get_bytes_copied(const copy_process_t* state);

size_t copy_process_get_files_processed(const copy_process_t* state);

unsigned int copy_process_get_overwrite(const copy_process_t* state);

unsigned int copy_process_get_progress(const copy_process_t* state);

/* Mutators */
void copy_process_increase_total_size(copy_process_t* state, size_t size);

void copy_process_increase_total_files(copy_process_t* state, size_t files);

void copy_process_increase_bytes_copied(copy_process_t* state, size_t bytes_copied);

void copy_process_increment_files_processed(copy_process_t* state);

/* Constructor */
copy_process_t* copy_process_init(unsigned int overwrite);

/* Destructor */
void copy_process_destroy(copy_process_t* state);

#endif
