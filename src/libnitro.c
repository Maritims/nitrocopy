#include "libnitro.h"
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFER_SIZE 4096
#define ERROR_MSG_SIZE 256

// State holder.
struct NitroCopyState {
    long long total_size;
    long long bytes_copied;
    char error_msg[ERROR_MSG_SIZE];
};

// Private helper function to translate errno to NitroStatus.
static NitroStatus nitro_status_from_errno(int err_val) {
    switch(err_val) {
        case ENOENT:
        case ENOTDIR:
        case EISDIR:
            return NITRO_ERROR_INVALID_PATH;
        case EACCES:
        case EPERM:
            return NITRO_ERROR_PERMISSION_DENIED;
        default:
            return NITRO_ERROR_GENERAL;
    }
}

// State constructor.
NitroCopyState* nitro_init() {
    NitroCopyState* state = malloc(sizeof(NitroCopyState));
    if(state == NULL) {
        return NULL;
    }

    state->total_size = 0;
    state->bytes_copied = 0;
    state->error_msg[0] = '\0'; // Initialize error message buffer.

    return state;
}

// State destructor.
void nitro_destroy(NitroCopyState* state) {
    if(state != NULL) {
        free(state);
    }
}

const char* nitro_get_last_error(const NitroCopyState* state) {
    if(state == NULL) {
        return "Invalid state handle";
    }
    return state->error_msg;
}

NitroStatus nitro_copy(NitroCopyState* state, const char* src, const char* dest) {
    struct stat file_stats;
    if(stat(src, &file_stats) == -1) {
        return NITRO_ERROR_OPENING_SRC_FILE;
    }

    state->total_size = nitro_get_total_size(src);
    const char* formatted_size = nitro_format_bytes(state->total_size);
    printf("Total size to copy: %s\n\n", formatted_size);

    if(S_ISDIR(file_stats.st_mode)) {
        return nitro_copy_directory(state, src, dest);
    } else {
        return nitro_copy_file(state, src, dest);
    }

    return NITRO_SUCCESS;
}

NitroStatus nitro_copy_file(NitroCopyState* state, const char *src, const char *dest) {
    // Return immediately if the source file doesn't exist.
	FILE *src_file = fopen(src, "rb");
	if(!src_file) {
        return NITRO_ERROR_OPENING_SRC_FILE;
	}

    // Create the destination directory if it doesn't exist.
	struct stat file_stats;
	if(stat(dest, &file_stats) == 0 && S_ISDIR(file_stats.st_mode)) {
		fclose(src_file);
        return nitro_status_from_errno(errno);
	}
 
    // Open or create the destination file if it doesn't exist. Any existing content is overwritten.
	FILE *dest_file = fopen(dest, "wb");
	if(!dest_file) {
		fclose(src_file);
		return nitro_status_from_errno(errno);
	}

    // Always go one line up to overwrite the total progress.
    printf("\033[A");
    printf("Copying file %s to %s...\n", src, dest);

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        if(fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            fprintf(stderr, "Error: Failed to write to destination file.\n");
            break;
        }
        state->bytes_copied += bytes_read;
        nitro_update_progress(state);
    }

    printf("\n");

    fclose(src_file);
    fclose(dest_file);

	return NITRO_SUCCESS;
}

NitroStatus nitro_copy_directory(NitroCopyState* state, const char* src, const char* dest) {
    DIR *dir_to_copy = opendir(src);
    if(!dir_to_copy) {
        return nitro_status_from_errno(errno);
    }

    // Reusable struct for controlling the file system status of different directories and files.
    struct stat file_stats;

    if(stat(dest, &file_stats) == -1) {
        if(mkdir(dest, 0755) == -1) {
            closedir(dir_to_copy);
            return nitro_status_from_errno(errno);
        }
    } else if(!S_ISDIR(file_stats.st_mode)) {
        closedir(dir_to_copy);
        return NITRO_ERROR_INVALID_PATH;
    }

    // Reusable struct for holding entires in the directory currently being copied.
    struct dirent *file_entry;
    NitroStatus current_status;

    while ((file_entry = readdir(dir_to_copy)) != NULL) {
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0) {
            // Stay in the current directory, don't go back up.
            continue;
        }

        char entry_source_path[1024];
        char entry_destination_path[1024];
        snprintf(entry_source_path, sizeof(entry_source_path), "%s/%s", src, file_entry->d_name);
        snprintf(entry_destination_path, sizeof(entry_destination_path), "%s/%s", dest, file_entry->d_name);

        if(stat(entry_source_path, &file_stats) == -1) {
            fprintf(stderr, "Warning: Could not get info for '%s', skipping.\n", src);
            continue;
        }

        if(S_ISDIR(file_stats.st_mode)) {
            current_status = nitro_copy_directory(state, entry_source_path, entry_destination_path);
        } else {
            current_status = nitro_copy_file(state, entry_source_path, entry_destination_path);
        }

        if(current_status != NITRO_SUCCESS) {
            break;
        }
    }

    closedir(dir_to_copy);
    return current_status;
}

long long nitro_get_total_size(const char* src) {
    struct stat file_stats;
    if(stat(src, &file_stats) == -1) {
        fprintf(stderr, "Error: Could not get file status for '%s': %s\n", src, strerror(errno));
        return -1;
    }

    if(S_ISDIR(file_stats.st_mode)) {
        long long total_size = 0;
        DIR *dir = opendir(src);
        if(!dir) {
            return 0;
        }

        struct dirent *entry;
        struct stat file_stats;
        char sub_path[1024];

        while ((entry = readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(sub_path, sizeof(sub_path), "%s/%s", src, entry->d_name);
            if(stat(sub_path, &file_stats) == 0) {
                if(S_ISDIR(file_stats.st_mode)) {
                    total_size += nitro_get_total_size(sub_path);
                } else {
                    total_size += file_stats.st_size;
                }
            }
        }

        closedir(dir);
        return total_size;
    } else {
        return file_stats.st_size;
    }
}

void nitro_update_progress(NitroCopyState* state) {
    if(state->total_size == 0) {
        return;
    }

    int progress = (int)((double) state->bytes_copied / state->total_size * 100);
    const char* formatted_bytes_copied = nitro_format_bytes(state->bytes_copied);
    const char* formatted_total_size = nitro_format_bytes(state->total_size);

    // Clear this line before printing the total progress.
    printf("\033[K");
    printf("Total progress: %d%% (%s/%s)", progress, formatted_bytes_copied, formatted_total_size);
    fflush(stdout);
}

char* nitro_format_bytes(long long bytes) {
    char* s = (char*)malloc(sizeof(char) * 20);
    const char* suffixes[] = {"B", "KB", "MB", "GB"};
    int i = 0;

    if(bytes == 0) {
        snprintf(s, 20, "0 B");
        return s;
    }

    i = (int)floor(log(bytes) / log(1024));
    i = (i < 0) ? 0 : i;

    if((unsigned long) i >= sizeof(suffixes) / sizeof(suffixes[0])) {
        i = sizeof(suffixes) / sizeof(suffixes[0]) - 1;
    }

    double value = (double)bytes / pow(1024, i);
    snprintf(s, 20, "%.2f %s", value, suffixes[i]);

    return s;
}
