#include "copy_process.h"

#include <stdlib.h>

struct copy_process_t {
    size_t total_size;
    size_t total_files;
    size_t bytes_copied;
    size_t files_processed;
    unsigned int overwrite;
};

size_t copy_process_get_total_size(const copy_process_t* state) {
    return state ? state->total_size : 0;
}

size_t copy_process_get_total_files(const copy_process_t* state) {
    return state ? state->total_files : 0;
}

size_t copy_process_get_bytes_copied(const copy_process_t* state) {
    return state ? state->bytes_copied : 0;
}

size_t copy_process_get_files_processed(const copy_process_t* state) {
    return state ? state->files_processed : 0;
}

unsigned int copy_process_get_overwrite(const copy_process_t* state) {
    return state ? state->overwrite : 0;
}

unsigned int copy_process_get_progress(const copy_process_t* state) {
    return state && state->total_size > 0 ? (unsigned int)((double) state->bytes_copied / state->total_size * 100) : 0;
}

void copy_process_increase_total_size(copy_process_t* state, size_t size) {
    if(state) {
        state->total_size += size;
    }
}

void copy_process_increase_total_files(copy_process_t* state, size_t files) {
    if(state) {
        state->total_files += files;
    }
}

void copy_process_increase_bytes_copied(copy_process_t* state, size_t bytes_copied) {
    if(state) {
        state->bytes_copied += bytes_copied;
    }
}

void copy_process_increment_files_processed(copy_process_t* state) {
    if(state) {
        state->files_processed++;
    }
}

copy_process_t* copy_process_init(unsigned int overwrite) {
    copy_process_t* state = malloc(sizeof(copy_process_t));
    if(!state) {
        return NULL;
    }

    state->total_size = 0;
    state->total_files = 0;
    state->bytes_copied = 0;
    state->files_processed = 0;
    state->overwrite = overwrite;

    return state;
}

void copy_process_destroy(copy_process_t* state) {
    if(state) {
        free(state);
    }
}
