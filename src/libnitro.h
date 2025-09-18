#ifndef LIBNITRO_H
#define LIBNITRO_H

// Opaque handling for holding state.
typedef struct NitroCopyState NitroCopyState;

// State management functions.
NitroCopyState* nitro_init();
void nitro_destroy(NitroCopyState* state);

// Core functionality for copying files and directories.
void nitro_copy(NitroCopyState* state, const char* src, const char* dest);
int nitro_copy_file(NitroCopyState* state, const char* src, const char* dest);
void nitro_copy_directory(NitroCopyState* state, const char* src, const char* dest);

// Utility functions.
long long nitro_get_total_size(const char* path);
void nitro_update_progress(NitroCopyState* state);
char* nitro_format_bytes(long long bytes);

#endif
