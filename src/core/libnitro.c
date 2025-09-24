#include "nitro_console.h"
#include "nitro_file.h"
#include "nitro_logging.h"
#include "libnitro.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define ERROR_MSG_SIZE 256

/* State holder.*/
struct NitroCopyState {
    unsigned long total_size;
    unsigned long total_files;
    unsigned long bytes_copied;
    unsigned long files_processed;
    unsigned int overwrite;
    char error_msg[ERROR_MSG_SIZE];
};

NitroCopyState* nitro_init(unsigned int overwrite) {
    NitroCopyState* state = malloc(sizeof(NitroCopyState));
    if(state == NULL) {
        return NULL;
    }

    state->total_size = 0;
    state->total_files = 0;
    state->bytes_copied = 0;
    state->files_processed = 0;
    state->overwrite = overwrite;
    state->error_msg[0] = '\0'; /* Initialize error message buffer. */    

    return state;
}

void nitro_destroy(NitroCopyState* state) {
    if(state != NULL) {
        free(state);
    }
}

void nitro_clear_line(int previous_line_length) {
    int line_width;
    int rows;

    line_width = nitro_console_get_width();
    
    if(previous_line_length > 0) {
        rows = (previous_line_length + line_width - 1) / line_width;
        nitro_console_move_cursor_up(rows);
        nitro_console_clear_from_cursor_down();
    }
}

NitroStatus nitro_copy(NitroCopyState* state, const char* src, const char* dest) {
    struct stat file_stats;
    char        options[64];
    const char* formatted_size;
    const char* formatted_bytes_copied;

    if(nitro_file_stat(src, &file_stats) != 0) {
        nitro_log_fatal("Failed to open src file \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return NITRO_ERROR_OPENING_FILE;
    }

    if(state->overwrite) {
        snprintf(options, sizeof(options), "overwrite = yes");
    } else {
        snprintf(options, sizeof(options), "overwrite = no");
    }

    nitro_log_info("Counting total number of files to copy...\n");
    nitro_get_total_stats(src, &state->total_size, &state->total_files); 
    formatted_size = nitro_format_bytes(state->total_size);
    nitro_log_info("Counted %lu files (%s)\n", state->total_files, formatted_size);

    if(S_ISDIR(file_stats.st_mode)) {
        NitroStatus status = nitro_copy_directory(state, src, dest);
        if(status != NITRO_SUCCESS) {
            return status;
        }
    } else {
        NitroStatus status = nitro_copy_file(state, src, dest);
        if(status != NITRO_SUCCESS) {
            return status;
        }
    }

    formatted_bytes_copied = nitro_format_bytes(state->bytes_copied);

    nitro_log_info("Finished copying files: %lu/%lu (%s/%s, %s)\n", state->files_processed, state->total_files, formatted_bytes_copied, formatted_size, options);

    return NITRO_SUCCESS;
}

NitroStatus nitro_copy_file(NitroCopyState* state, const char *src, const char *dest) {
	struct stat file_stats;
	FILE        *src_file;
    FILE        *dest_file;
    char        buffer[BUFFER_SIZE];
    size_t      bytes_read;
    size_t      total_bytes_read;
    int         last_reported_percentage_block;
    int         current_percentage_block;
    int         percentage_block_divisor;
    int         foo;

    src_file = fopen(src, "rb");
	if(!src_file) {
        nitro_log_error("Failed to open src file: \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return NITRO_ERROR_OPENING_FILE; 
	}

	if(nitro_file_stat(dest, &file_stats) == 0) {
        if(S_ISDIR(file_stats.st_mode)) {
		    fclose(src_file);
            nitro_log_error("Destination path \"%s\" is a directory\n", dest);
            return NITRO_ERROR_DEST_IS_DIR;
        }

        if(!state->overwrite) {
            while(1) {
                char response[4];

                printf("Destination file \"%s\" exists already. Overwrite? (y/n): ", dest);
                fflush(stdout);

                if(fgets(response, sizeof(response), stdin) != NULL) {
                    if(response[0] == 'y' || response[0] == 'Y') {
                        printf("Overwriting file \"%s\"\n", dest);
                        break;
                    }
                    if(response[0] == 'n' || response[0] == 'N') {
                        return NITRO_SUCCESS;
                    }
                }
                printf("Invalid input. Please enter 'y' or 'n'.\n");
            }
        }
	}
 
	dest_file = fopen(dest, "wb");
	if(!dest_file) {
		fclose(src_file);
        nitro_log_error("Failed to open or create destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
        return NITRO_ERROR_OPENING_FILE;
	}

    state->files_processed++;
    last_reported_percentage_block  = -1;
    total_bytes_read                = 0;
    foo = (file_stats.st_size / BUFFER_SIZE) / BUFFER_SIZE;
    percentage_block_divisor        = 100 / foo;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        unsigned int    progress;
        const char*     formatted_bytes_copied;
        const char*     formatted_total_size;

        if(fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            nitro_log_error("Failed to write to destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
            break;
        }

        state->bytes_copied         += bytes_read;
        total_bytes_read += bytes_read;
        progress                    = (int)((double) total_bytes_read / file_stats.st_size * 100);
        current_percentage_block    = progress / percentage_block_divisor;

        if(current_percentage_block > last_reported_percentage_block) {
            formatted_bytes_copied      = nitro_format_bytes(state->bytes_copied);
            formatted_total_size        = nitro_format_bytes(state->total_size);

            nitro_log_info("Copying file %lu/%lu: %s -> %s (%3d%%, %s/%s)\n", state->files_processed, state->total_files, src, dest, progress, formatted_bytes_copied, formatted_total_size);
            fflush(stdout);
            last_reported_percentage_block = current_percentage_block;
        }
    }

    fclose(src_file);
    fclose(dest_file);

	return NITRO_SUCCESS;
}

NitroStatus nitro_copy_directory(NitroCopyState* state, const char* src, const char* dest) {
    DIR             *dir_to_copy;
    struct stat     file_stats;
    struct dirent   *file_entry;
    NitroStatus     current_status;

    dir_to_copy = nitro_file_opendir(src);
    if(!dir_to_copy) {
        nitro_log_error("Failed to open src dir \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return NITRO_ERROR_INVALID_PATH;
    }

    if(nitro_file_stat(dest, &file_stats) != NITRO_SUCCESS) {
        if(nitro_file_mkdir(dest, 0755) == -1) {
            nitro_file_closedir(dir_to_copy);
            nitro_log_error("Failed to create destination directdory \"%s\": %d (%s)\n", dest, errno, strerror(errno));
            return NITRO_ERROR_CREATING_DIR;
        }
    } else if(!S_ISDIR(file_stats.st_mode)) {
        nitro_file_closedir(dir_to_copy);
        nitro_log_error("Destination path \"%s\" is not a directory\n", dest);
        return NITRO_ERROR_INVALID_PATH;
    }

    while ((file_entry = nitro_file_readdir(dir_to_copy)) != NULL) {
        char entry_source_path[1024];
        char entry_destination_path[1024];
        
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0) {
            /* Stay in the current directory, don't go back up. */
            continue;
        }

        snprintf(entry_source_path, sizeof(entry_source_path), "%s/%s", src, file_entry->d_name);
        snprintf(entry_destination_path, sizeof(entry_destination_path), "%s/%s", dest, file_entry->d_name);

        if(nitro_file_stat(entry_source_path, &file_stats) != NITRO_SUCCESS) {
            nitro_log_error("Warning: Could not get info for '%s', skipping.\n", src);
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

    nitro_file_closedir(dir_to_copy);
    return current_status;
}

NitroStatus nitro_get_total_stats(const char* src, unsigned long* total_size, unsigned long* file_count) {
    struct stat     file_stats;
    struct dirent*  entry;
    char            sub_path[1024];

    if(nitro_file_stat(src, &file_stats) != NITRO_SUCCESS) {
        nitro_log_error("Failed to acquire file status for src path \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return NITRO_ERROR_OPENING_FILE;
    }

    if(S_ISDIR(file_stats.st_mode)) {
        DIR *dir = nitro_file_opendir(src);
        if(!dir) {
            nitro_log_error("Failed to open src dir \"%s\": %d (%s)\n", src, errno, strerror(errno));
            return NITRO_ERROR_OPENING_FILE;
        }


        while ((entry = nitro_file_readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            snprintf(sub_path, sizeof(sub_path), "%s/%s", src, entry->d_name);
            if(nitro_file_stat(sub_path, &file_stats) == 0) {
                if(S_ISDIR(file_stats.st_mode)) {
                    nitro_get_total_stats(sub_path, total_size, file_count);
                } else {
                    *total_size += file_stats.st_size;
                    *file_count += 1;
                }
            }
        }

        nitro_file_closedir(dir);
    } else {
        *total_size = file_stats.st_size;
        *file_count += 1;
    }

    return NITRO_SUCCESS;
}

void nitro_update_progress(NitroCopyState* state) {
    unsigned int    progress;
    const char*     formatted_bytes_copied;
    const char*     formatted_total_size;

    if(state->total_size == 0) {
        return;
    }

    progress                = (unsigned int)((double) state->bytes_copied / state->total_size * 100);
    formatted_bytes_copied  = nitro_format_bytes(state->bytes_copied);
    formatted_total_size    = nitro_format_bytes(state->total_size);

    
    nitro_clear_line(0);

    printf("\rTotal progress: %d%% (%s/%s)\n", progress, formatted_bytes_copied, formatted_total_size);
    fflush(stdout);
}

char* nitro_format_bytes(unsigned long bytes) {
    char* s = (char*)malloc(sizeof(char) * 20);
    const char* suffixes[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double value;

    if(bytes == 0) {
        snprintf(s, 20, "0 B");
        return s;
    }

    i = (int)floor(log(bytes) / log(1024));
    i = (i < 0) ? 0 : i;

    if((unsigned long) i >= sizeof(suffixes) / sizeof(suffixes[0])) {
        i = sizeof(suffixes) / sizeof(suffixes[0]) - 1;
    }

    value = (double)bytes / pow(1024, i);
    snprintf(s, 20, "%.2f %s", value, suffixes[i]);

    return s;
}
