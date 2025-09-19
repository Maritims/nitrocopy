#ifndef LIBNITRO_H
#define LIBNITRO_H

// Opaque handling for holding state.
typedef struct NitroCopyState NitroCopyState;

// Define status codes.
typedef enum {
    NITRO_SUCCESS = 0,
    NITRO_ERROR_OPENING_SRC_FILE,
    NITRO_ERROR_DEST_IS_DIR,
    NITRO_ERROR_DIR_CREATE,
    NITRO_ERROR_IO,
    NITRO_ERROR_INVALID_PATH,
    NITRO_ERROR_PERMISSION_DENIED,
    NITRO_ERROR_GENERAL
} NitroStatus;
    

// State management functions.
NitroCopyState* nitro_init();
void nitro_destroy(NitroCopyState* state);

// Core functionality for copying files and directories.
NitroStatus nitro_copy(NitroCopyState* state, const char* src, const char* dest);
NitroStatus nitro_copy_file(NitroCopyState* state, const char* src, const char* dest);
NitroStatus nitro_copy_directory(NitroCopyState* state, const char* src, const char* dest);

// Utility functions.
long long nitro_get_total_size(const char* path);
void nitro_update_progress(NitroCopyState* state);
char* nitro_format_bytes(long long bytes);
const char* nitro_get_last_error(const NitroCopyState* state);

#endif
