#include "copier.h"
#include "copy_process.h"
#include "file.h"
#include "logging.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 4096
#define ERROR_MSG_SIZE 256

static char* copier_format_bytes(size_t bytes) {
    char* s = (char*)malloc(sizeof(char) * 20);
    const char* suffixes[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double value;

    if(bytes == 0) {
        sprintf(s, "0 B");
        return s;
    }

    i = (int)floor(log(bytes) / log(1024));
    i = (i < 0) ? 0 : i;

    if((unsigned long) i >= sizeof(suffixes) / sizeof(suffixes[0])) {
        i = sizeof(suffixes) / sizeof(suffixes[0]) - 1;
    }

    value = (double)bytes / pow(1024, i);
    sprintf(s, "%.2f %s", value, suffixes[i]);

    return s;
}

int copier_copy(const char* src, const char* dest, copy_process_t* copy_process) {
    struct stat file_stats;
    char        options[64];
    const char* formatted_size;
    const char* formatted_bytes_copied;

    if(compat_stat(src, &file_stats) != 0) {
        log_fatal("Failed to open src file \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return 1;
    }

    if(copy_process_get_overwrite(copy_process) == 1) {
        sprintf(options, "overwrite = yes");
    } else {
        sprintf(options, "overwrite = no");
    }

    log_info("Counting total number of files to copy...\n");
    copier_get_total_stats(src, copy_process);
    formatted_size = copier_format_bytes(copy_process_get_total_size(copy_process));
    log_info("Counted %lu files (%s)\n", copy_process_get_total_files(copy_process), formatted_size);

    if(S_ISDIR(file_stats.st_mode)) {
        int status = copier_copy_dir(src, dest, copy_process);
        if(status != 0) {
            return status;
        }
    } else {
        int status = copier_copy_file(src, dest, copy_process);
        if(status != 0) {
            return status;
        }
    }

    formatted_bytes_copied = copier_format_bytes(copy_process_get_bytes_copied(copy_process));

    log_info("Finished copying files: %lu/%lu (%s/%s, %s)\n", copy_process_get_files_processed(copy_process), copy_process_get_total_files(copy_process), formatted_bytes_copied, formatted_size, options);

    return 0;
}

int copier_copy_file(const char* src, const char* dest, copy_process_t* state) {
	struct stat file_stats;
	FILE        *src_file;
    FILE        *dest_file;
    char        buffer[BUFFER_SIZE];
    size_t      bytes_read;
    size_t      total_bytes_read;
    time_t      start_time;

    src_file = fopen(src, "rb");
	if(!src_file) {
        log_error("Failed to open src file: \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return 1; 
	}

	if(compat_stat(dest, &file_stats) == 0) {
        if(S_ISDIR(file_stats.st_mode)) {
		    fclose(src_file);
            log_error("Destination path \"%s\" is a directory\n", dest);
            return 2;
        }

        if(!copy_process_get_overwrite(state)) {
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
                        return 0;
                    }
                }
                printf("Invalid input. Please enter 'y' or 'n'.\n");
            }
        }
	}
 
	dest_file = fopen(dest, "wb");
	if(!dest_file) {
		fclose(src_file);
        log_error("Failed to open or create destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
        return 1;
	}

    copy_process_increment_files_processed(state);
    total_bytes_read    = 0;
    start_time          = time(NULL);

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        unsigned int    progress;
        const char*     formatted_bytes_copied;
        const char*     formatted_total_size;
        time_t          now;

        if(fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            log_error("Failed to write to destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
            break;
        }

        copy_process_increase_bytes_copied(state, bytes_read);
        total_bytes_read    += bytes_read;
        progress            = (int)((double) total_bytes_read / file_stats.st_size * 100);
        now                 = time(NULL);

        if(difftime(now, start_time) >= 10.0 || progress == 100.0) {
            formatted_bytes_copied      = copier_format_bytes(copy_process_get_bytes_copied(state));
            formatted_total_size        = copier_format_bytes(copy_process_get_total_size(state));

            log_info("Copying file %lu/%lu: %s -> %s (%3d%%, %s/%s)\n", copy_process_get_files_processed(state), copy_process_get_total_files(state), src, dest, progress, formatted_bytes_copied, formatted_total_size);
            fflush(stdout);
        }
    }

    fclose(src_file);
    fclose(dest_file);

	return 0;
}

int copier_copy_dir(const char* src, const char* dest, copy_process_t* state) {
    DIR             *dir_to_copy;
    struct stat     file_stats;
    struct dirent   *file_entry;
    int             current_status;

    dir_to_copy = compat_opendir(src);
    if(!dir_to_copy) {
        log_error("Failed to open src dir \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return 3;
    }

    if(compat_stat(dest, &file_stats) != 0) {
        if(compat_mkdir(dest, 0755) == -1) {
            compat_closedir(dir_to_copy);
            log_error("Failed to create destination directdory \"%s\": %d (%s)\n", dest, errno, strerror(errno));
            return 4;
        }
    } else if(!S_ISDIR(file_stats.st_mode)) {
        compat_closedir(dir_to_copy);
        log_error("Destination path \"%s\" is not a directory\n", dest);
        return 5;
    }

    while ((file_entry = compat_readdir(dir_to_copy)) != NULL) {
        char entry_source_path[1024];
        char entry_destination_path[1024];
        
        if(strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0) {
            /* Stay in the current directory, don't go back up. */
            continue;
        }

        sprintf(entry_source_path, "%s/%s", src, file_entry->d_name);
        sprintf(entry_destination_path, "%s/%s", dest, file_entry->d_name);

        if(compat_stat(entry_source_path, &file_stats) != 0) {
            log_error("Warning: Could not get info for '%s', skipping.\n", src);
            continue;
        }

        if(S_ISDIR(file_stats.st_mode)) {
            current_status = copier_copy_dir(entry_source_path, entry_destination_path, state);
        } else {
            current_status = copier_copy_file(entry_source_path, entry_destination_path, state);
        }

        if(current_status != 0) {
            break;
        }
    }

    compat_closedir(dir_to_copy);
    return current_status;
}

/** 
 * copy_get_total_stats: Populate the copy process with the total number of files and bytes to copy.
 * @src The path to the file or dir being traversed.
 * @copy_process A pointer to the current copy process (cannot be NULL).
 *
 * Returns a code indicating the outcome of the operation:
 * 0 = Success.
 * 1 = Copy process pointer is NULL.
 * 2 = Failed to acquire stats for src. The resource probably doesn't exist.
 * 3 = Failed to open src dir.
 */
int copier_get_total_stats(const char* src, copy_process_t* copy_process) {
    struct stat     file_stats;
    struct dirent*  entry;
    char            sub_path[1024];

    if(!copy_process) {
        return 1;
    }

    if(compat_stat(src, &file_stats) != 0) {
        return 2;
    }

    if(S_ISDIR(file_stats.st_mode)) {
        DIR *dir = compat_opendir(src);
        if(!dir) {
            return 3;
        }


        while ((entry = compat_readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            sprintf(sub_path, "%s/%s", src, entry->d_name);
            if(compat_stat(sub_path, &file_stats) == 0) {
                if(S_ISDIR(file_stats.st_mode)) {
                    copier_get_total_stats(sub_path, copy_process);
                } else {
                    copy_process_increase_total_size(copy_process, file_stats.st_size);
                    copy_process_increase_total_files(copy_process, 1);
                }
            }
        }

        compat_closedir(dir);
    } else {
        copy_process_increase_total_size(copy_process, file_stats.st_size);
        copy_process_increase_total_files(copy_process, 1);
    }

    return 0;
}

