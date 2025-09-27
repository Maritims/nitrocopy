#include "copier.h"
#include "file.h"
#include "logging.h"
#include "runtime.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 4096

static void format_bytes(size_t bytes, char* buffer) {
    const char* suffixes[] = {"B", "KB", "MB", "GB"};
    int i = 0;
    double value;

    if(bytes == 0) {
        sprintf(buffer, "0 B");
        return;
    }

    i = (int)floor(log(bytes) / log(1024));
    i = (i < 0) ? 0 : i;

    if((unsigned long) i >= sizeof(suffixes) / sizeof(suffixes[0])) {
        i = sizeof(suffixes) / sizeof(suffixes[0]) - 1;
    }

    value = (double)bytes / pow(1024, i);
    sprintf(buffer, "%.2f %s", value, suffixes[i]);
}

/**
 * copier_overwrite: Checks whether a file can be and will be overwritten.
 * @dest: The path to the file that may or may not be overwritten.
 * @copy_process: A pointer to the current copy process (cannot be NULL).
 *
 * Returns a code indicating whether the file should be overwritten:
 * 0 = The file should be overwritten.
 * 1 = The file should not be overwritten, or overwriting is not enabled in the ongoing copy process.
 * 2 = No stats could be retrieved for the destination path.
 * 3 = The destination path represents a dir, not a file.
 */
static int determine_overwrite(const char* dest) {
    struct stat stats;

    if(overwrite == 1) {
        return 0;
    }

    if(compat_stat(dest, &stats) != 0) {
        return 0;
    }
        
    if(S_ISDIR(stats.st_mode)) {
        fprintf(stderr, "Dest %s is dir\n", dest);
        return -1;
    }

    while(1) {
        char response[4];

        printf("Destination file \"%s\" exists already. Overwrite? (y/n): ", dest);
        fflush(stdout);

    
        if(fgets(response, sizeof(response), stdin) != NULL) {
            if(response[0] == 'y' || response[0] == 'Y') {
                return 0;
            }

            if(response[0] == 'n' || response[0] == 'N') {
                return 1;
            }
        }

        fprintf(stderr, "Invalid input. Please enter 'y' or 'n'.\n");
    }
}

int copier_copy(const char* src, const char* dest, size_t total_files, size_t total_bytes, size_t* total_files_copied, size_t* total_bytes_copied) {
    size_t      total_progress  = 0;
    size_t      bytes_copied    = 0;
    char        formatted_bytes_copied[256];
    char        formatted_total_bytes[256];

    struct stat     stats;
    struct dirent*  src_file;
    DIR*            src_dir;

    if(compat_stat(src, &stats) != 0) {
        fprintf(stderr, "Failed to acquire stats for %s: %d (%s)\n", src, errno, strerror(errno));
        return 1;
    }

    if(S_ISDIR(stats.st_mode)) {
        src_dir = compat_opendir(src);
        if(!src_dir) {
            fprintf(stderr, "Failed to open src dir %s: %d (%s)\n", src, errno, strerror(errno));
            return 1;
        }

        if(compat_stat(dest, &stats) != 0 && compat_mkdir(dest, 0755) == -1) {
            compat_closedir(src_dir);
            fprintf(stderr, "Failed to create directory %s: %d (%s)\n", dest, errno, strerror(errno));
            return 1;
        }

        while((src_file = compat_readdir(src_dir)) != NULL) {
            char foo[1024];
            char bar[1024];

            if(strcmp(src_file->d_name, ".") == 0 || strcmp(src_file->d_name, "..") == 0) {
                continue;
            }

            sprintf(foo, "%s/%s", src, src_file->d_name);
            sprintf(bar, "%s/%s", dest, src_file->d_name);

            if(copier_copy(foo, bar, total_files, total_bytes, total_files_copied, total_bytes_copied) != 0) {
                return 1;
            }
        }

        free(src_dir);
    } else {
        *total_files_copied += 1;

        log_info("Copying file %lu/%lu: %s -> %s\n", *total_files_copied, total_files, src, dest);

        if((bytes_copied = copier_copy_file(src, dest)) >= 0) {
            *total_bytes_copied += bytes_copied;

            total_progress = *total_bytes_copied == 0 ? 0 : (int)((double) *total_bytes_copied / (double) total_bytes * 100.0);
        
            format_bytes(*total_bytes_copied, formatted_bytes_copied);
            format_bytes(total_bytes, formatted_total_bytes);

            log_info("-> Total progress: %3d%%, %s/%s\n\n", total_progress, formatted_bytes_copied, formatted_total_bytes);
        } else {
            return 1;
        }
    }

    return 0;
}

int copier_execute(const char* src, const char* dest) {
    size_t total_files  = 0;
    size_t total_bytes  = 0;
    size_t files_copied = 0;
    size_t bytes_copied = 0;
    char   formatted_total_bytes[256];
    char   formatted_bytes_copied[256];

    printf("Counting total number of files to copy...\n");

    if(copier_get_total_stats(src, &total_files, &total_bytes) != 0) {
        fprintf(stderr, "copier_copy(): Failed to calculate total number of files to copy.\n");
        return 1;
    }

    if(total_bytes == 0) {
        fprintf(stderr, "There's nothing to copy.\n");
        return 1;
    }

    format_bytes(total_bytes, formatted_total_bytes);
    printf("Counted %lu files (%s)\n", (unsigned long) total_files, formatted_total_bytes);

    copier_copy(src, dest, total_files, total_bytes, &files_copied, &bytes_copied);

    format_bytes(bytes_copied, formatted_bytes_copied);
    format_bytes(total_bytes, formatted_total_bytes);
    printf("Finished copying files: %lu/%lu (%s/%s)\n", (unsigned long) files_copied, (unsigned long) total_files, formatted_bytes_copied, formatted_total_bytes);

    return 0;
}

size_t copier_copy_file(const char* src, const char* dest) {
	struct stat stats;
	FILE        *src_file;
    FILE        *dest_file;
    char        buffer[BUFFER_SIZE];
    size_t      bytes_read;
    size_t      total_bytes_read;
    time_t      start_time;

    if(compat_stat(src, &stats) != 0) {
        fprintf(stderr, "Failed to acquire stats for %s: %d (%s)\n", dest, errno, strerror(errno));
        return -1;
    }

    src_file = fopen(src, "rb");
	if(!src_file) {
        fprintf(stderr, "Failed to open src %s: %d (%s)\n", src, errno, strerror(errno));
        return -1;
	}

    if(S_ISDIR(stats.st_mode)) {
        fclose(src_file);
        fprintf(stderr, "Dest %s is a directory\n", dest);
        return -1;
    }

    if(determine_overwrite(dest) != 0) {
        return 0;
    }
 
	dest_file = fopen(dest, "wb");
	if(!dest_file) {
		fclose(src_file);
        fprintf(stderr, "Failed to open or create destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
        return -1;
	}

    start_time          = time(NULL);
    total_bytes_read     = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src_file)) > 0) {
        unsigned int    file_progress;
        time_t          now;
        char            formatted_total_bytes_read[256];
        char            formatted_file_size[256];

        if(fwrite(buffer, 1, bytes_read, dest_file) != bytes_read) {
            fprintf(stderr, "Failed to write to destination file \"%s\": %d (%s)\n", dest, errno, strerror(errno));
            break;
        }

        total_bytes_read    += bytes_read;
        file_progress       = (int)((double) total_bytes_read / stats.st_size * 100);
        now                 = time(NULL);

        format_bytes(total_bytes_read, formatted_total_bytes_read);
        format_bytes(stats.st_size, formatted_file_size);

        if(difftime(now, start_time) >= 10.0 || file_progress == 100.0) {
            log_info("-> File progress : %3d%%, %s/%s\n", file_progress, formatted_total_bytes_read, formatted_file_size);
        }
    }

    fclose(src_file);
    fclose(dest_file);

	return total_bytes_read;
}

int copier_get_total_stats(const char* src, size_t* total_files, size_t* total_bytes) {
    struct stat     stats;
    struct dirent*  entry;
    char            sub_path[1024];

    if(!total_files) {
        fprintf(stderr, "total_files cannot be NULL\n");
        return 1;
    }

    if(!total_bytes) {
        fprintf(stderr, "total_bytes cannot be NULL\n");
        return 1;
    }


    if(compat_stat(src, &stats) != 0) {
        fprintf(stderr, "Failed to acquire stats for source \"%s\": %d (%s)\n", src, errno, strerror(errno));
        return 1;
    }

    if(S_ISDIR(stats.st_mode)) {
        DIR *dir = compat_opendir(src);
        if(!dir) {
            fprintf(stderr, "Failed to open %s: %d (%s)\n", src, errno, strerror(errno));
            return 1;
        }

        while ((entry = compat_readdir(dir)) != NULL) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            sprintf(sub_path, "%s/%s", src, entry->d_name);
            if(compat_stat(sub_path, &stats) != 0) {
                fprintf(stderr, "Failed to acquire file stats for sub path %s: %d (%s)\n", sub_path, errno, strerror(errno));
                return 1;
            }

             if(S_ISDIR(stats.st_mode)) {
                copier_get_total_stats(sub_path, total_files, total_bytes);
            } else {
                *total_files    += 1;
                *total_bytes    += stats.st_size;
            }
        }

        compat_closedir(dir);
    } else {
        *total_files    += 1;
        *total_bytes    += stats.st_size;
    }

    return 0;
}

