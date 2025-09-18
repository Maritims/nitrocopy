#include "libnitro.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#define BUFFER_SIZE 4096

// State holder.
struct NitroCopyState {
    long long total_size;
    long long bytes_copied;
};

// State constructor.
NitroCopyState* nitro_init() {
    NitroCopyState* state = malloc(sizeof(NitroCopyState));
    if(state == NULL) {
        return NULL;
    }

    state->total_size = 0;
    state->bytes_copied = 0;

    return state;
}

// State destructor.
void nitro_destroy(NitroCopyState* state) {
    if(state != NULL) {
        free(state);
    }
}

void nitro_copy(NitroCopyState* state, const char* src, const char* dest) {
    struct stat file_stats;
    if(stat(src, &file_stats) == -1) {
        fprintf(stderr, "Error: Could not get file status for src '%s': %s\n", src, strerror(errno));
        return;
    }

    state->total_size = nitro_get_total_size(src);
    const char* formatted_size = nitro_format_bytes(state->total_size);
    printf("Total size to copy: %s\n", formatted_size);

    if(S_ISDIR(file_stats.st_mode)) {
        nitro_copy_directory(state, src, dest);
    } else {
        nitro_copy_file(state, src, dest);
    }

    printf("\nCopying complete!\n");
}

int nitro_copy_file(NitroCopyState* state, const char *src, const char *dest) {
    // Return immediately if the source file doesn't exist.
	FILE *src_file = fopen(src, "rb");
	if(!src_file) {
		fprintf(stderr, "Error: Could not open source file '%s': %s\n", src, strerror(errno));
		return -1;
	}

    // Create the destination directory if it doesn't exist.
	struct stat file_stats;
	if(stat(dest, &file_stats) == 0 && S_ISDIR(file_stats.st_mode)) {
		fprintf(stderr, "Error: Destination '%s' is a directory. Cannot overwrite with file.\n", dest);
		fclose(src_file);
		return -1;
	}
 
    // Open or create the destination file if it doesn't exist. Any existing content is overwritten.
	FILE *dest_file = fopen(dest, "wb");
	if(!dest_file) {
		fprintf(stderr, "Error: Could not open destination file '%s': %s\n", dest, strerror(errno));
		fclose(src_file);
		return -1;
	}

    printf("\nCopying file %s to %s...\n", src, dest);

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

    fclose(src_file);
    fclose(dest_file);

	return 0;
}

void nitro_copy_directory(NitroCopyState* state, const char* src, const char* dest) {
    printf("Entering directory %s...\n", src);

    DIR *dir_to_copy = opendir(src);
    if(!dir_to_copy) {
        fprintf(stderr, "Error: Could not open source directory '%s': %s\n", src, strerror(errno));
        return;
    }

    // Reusable struct for controlling the file system status of different directories and files.
    struct stat file_stats;

    if(stat(dest, &file_stats) == -1) {
        if(mkdir(dest, 0755) == -1) {
            fprintf(stderr, "Error: Could not create destination directory '%s': %s\n", dest, strerror(errno));
            closedir(dir_to_copy);
            return;
        }
    } else if(!S_ISDIR(file_stats.st_mode)) {
        fprintf(stderr, "Error: Destination '%s' is not a directory.\n", dest);
        closedir(dir_to_copy);
        return;
    }

    // Reusable struct for holding entires in the directory currently being copied.
    struct dirent *file_entry;

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
            nitro_copy_directory(state, entry_source_path, entry_destination_path);
        } else {
            nitro_copy_file(state, entry_source_path, entry_destination_path);
        }
    }

    closedir(dir_to_copy);
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

    printf("\rTotal progress: %d%% (%s/%s)", progress, formatted_bytes_copied, formatted_total_size);
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

    if(i >= sizeof(suffixes) / sizeof(suffixes[0])) {
        i = sizeof(suffixes) / sizeof(suffixes[0]) - 1;
    }

    double value = (double)bytes / pow(1024, i);
    snprintf(s, 20, "%.2f %s", value, suffixes[i]);

    return s;
}
