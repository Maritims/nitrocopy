#include "copier.h"
#include "file.h"
#include "fletcher32.h"
#include "logging.h"
#include "word16.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CHUNK_SIZE 65535

/*
 * Prompt the user to choose whether to resume, overwrite or skip the file when it exists.
 */
static copy_existing_action_t prompt_user_action(const char* dst) {
    char input[16];

    if(g_default_copy_action != COPY_ACTION_UNSET) {
        return g_default_copy_action;
    }

    while(1) {
        printf("Destination file \"%s\" exists. Choose action: [R]esume, [O]verwrite, [S]kip: ", dst);

        if(!fgets(input, sizeof(input), stdin)) {
            continue;
        }

        if(input[0] == 'R' || input[0] == 'r') {
            return COPY_RESUME;
        }
        if(input[0] == 'O' || input[0] == 'o') {
            return COPY_OVERWRITE;
        }
        if(input[0] == 'S' || input[0] == 's') {
            return COPY_SKIP;
        }
    }
}

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

/**
 * log_progress(): Logs progress once every 10 seconds or when we've copied everything.
 */
static void log_progress(size_t copied, size_t total, time_t start, time_t* last_log_time) {
    time_t  now = time(NULL);
    double  elapsed;
    double  speed;
    int     percent;
    char    formatted_copied[64];
    char    formatted_total[64];

    if(difftime(now, *last_log_time) >= 10.0 || copied == total) {
        elapsed = difftime(now, start);
        speed   = elapsed > 0 ? ((double) total / elapsed) : 0.0;

        percent = (int)(((double) copied / (double) total) * 100.0);
        format_bytes(copied, formatted_copied);
        format_bytes(total, formatted_total);

        log_info("-> File progress: %3d%%, %s/%s, Speed: %.2f MB/s\n", percent, formatted_copied, formatted_total, speed);

        *last_log_time = now;
    }
}

static FILE* open_src_file(const char* src) {
    FILE* src_file = fopen(src, "rb");
	if(!src_file) {
        fprintf(stderr, "Error opening source file %s: %d (%s)\n", src, errno, strerror(errno));
	}
    return src_file;
}

static FILE* open_dst_file(const char* dst) {
    FILE*       dst_file;
    struct stat dst_stats;

    if(stat(dst, &dst_stats) == 0) {
        log_info("Destination file \"%s\" already exists.\n", dst);
    }

	dst_file = fopen(dst, "r+b");
	if(!dst_file) {
        dst_file = fopen(dst, "w+b");
        if(!dst_file) {
            fprintf(stderr, "Error opening destination file: %s: %d (%s)\n", dst, errno, strerror(errno));
        }
	}
    return dst_file;
}

/*
 * process_chunk(): Read from the source file and write to the destination file. Uses Fletcher-32 to understand whether there's a difference between the source and destination chunks.
 * @returns 0 if EOF is reached, -1 if an error occurs, 1 if the chunk is written.
 */
static int process_chunk(FILE* src_file, FILE* dst_file, size_t chunk_index, size_t* bytes_processed) {
    unsigned char   buffer[CHUNK_SIZE];
    unsigned short  words[CHUNK_SIZE / 2 + 1];
    size_t          src_bytes;
    size_t          dst_bytes;
    size_t          word_count;
    unsigned long   checksum_src;
    unsigned long   checksum_dst;

    /* Read from source and calculate checksum */
    src_bytes = fread(buffer, 1, CHUNK_SIZE, src_file);
    if(src_bytes == 0) {
        *bytes_processed = 0;
        return feof(src_file) ? 0 : -1;
    }
    word_count      = word16_create(buffer, src_bytes, words);
    checksum_src    = fletcher32(words, word_count);

    /* Read from destination and calculate checksum */
    fseek(dst_file, (long)(chunk_index * CHUNK_SIZE), SEEK_SET);
    dst_bytes       = fread(buffer, 1, src_bytes, dst_file);
    word_count      = word16_create(buffer, dst_bytes, words);
    checksum_dst    = fletcher32(words, word_count);

    /* Rewrite the chunk if different */
    if(checksum_src != checksum_dst) {
        log_info("Checksum mismatch at chunk index %lu. Rewriting chunk");

        fseek(src_file, (long)(chunk_index * CHUNK_SIZE), SEEK_SET);
        src_bytes = fread(buffer, 1, CHUNK_SIZE, src_file);

        fseek(dst_file, (long)(chunk_index * CHUNK_SIZE), SEEK_SET);
        if(fwrite(buffer, 1, src_bytes, dst_file) != src_bytes) {
            fprintf(stderr, "Failed to write chunk: %lu: %d (%s)\n", (unsigned long) chunk_index, errno, strerror(errno));
            return -1;
        }
        fflush(dst_file);
    }

    *bytes_processed = src_bytes;
    return 1;
}

/*
 * process_chunks(): Main copy loop.
 * @returns The total bytes copied.
*/
static size_t process_chunks(FILE* src_file, FILE* dst_file, const struct stat* src_stats) {
    size_t  total_bytes_copied   = 0;
    size_t  chunk_index          = 0;
    size_t  bytes_processed;
    int     status;

    time_t start_time       = time(NULL);
    time_t last_log_time    = start_time;

    while((status = process_chunk(src_file, dst_file, chunk_index, &bytes_processed)) > 0) {
        total_bytes_copied += bytes_processed;
        log_progress(total_bytes_copied, src_stats->st_size, start_time, &last_log_time);
        chunk_index++;
    }

    return total_bytes_copied;
}

static int get_file_stats(const char* path, struct stat* stats) {
    if(compat_stat(path, stats) != 0) {
        fprintf(stderr, "Failed to stat file %s: %d (%s)\n", path, errno, strerror(errno));
        return -1;
    }
    return 0;
}

/*
 * determine_resume_chunk(): Determine which chunk to resume the copy operation from.
 * @returns The chunk to resume from.
 */
static size_t determine_resume_chunk(FILE* src_file, FILE* dst_file, const char* dst) {
    struct stat dst_stats;
    size_t      resume_chunk    = 0;
    size_t      check_chunk     = 0;
    size_t      dst_size        = 0;
    int         status          = 0;
    size_t      bytes_processed = 0;

    if(stat(dst, &dst_stats) != 0) {
        return resume_chunk;
    }

    dst_size        = dst_stats.st_size;
    resume_chunk    = dst_size / CHUNK_SIZE;

    if(resume_chunk <= 0) {
        return resume_chunk;
    }

    check_chunk = resume_chunk - 1;
    
    fseek(src_file, (long)(check_chunk * CHUNK_SIZE), SEEK_SET);
    fseek(dst_file, (long)(check_chunk * CHUNK_SIZE), SEEK_SET);

    status = process_chunk(src_file, dst_file, check_chunk, &bytes_processed);

    if(status < 0) {
        fprintf(stderr, "Error verifying chunk %lu\n", (unsigned long) check_chunk);
        return 0; /* We'll interpret 0 as a signal to just restart since we couldn't verify the chunk. */
    }

    if(status == 1) {
        log_info("Determined resume chunk to be at index %lu\n", check_chunk);
        resume_chunk = check_chunk; /* When we encounter a mismatch it's best to roll back to the previous chunk. */
    }

    return resume_chunk;
}

/*
 * reposition_streams_for_resume(): Set the stream positions of the src_file and dst_file streams to resume_chunk.
 */
static void reposition_streams_for_resume(FILE* src_file, FILE* dst_file, size_t resume_chunk) {
    long offset = (long)(resume_chunk * CHUNK_SIZE);
    fseek(src_file, offset, SEEK_SET);
    fseek(dst_file, offset, SEEK_SET);
}

size_t copier_copy_file(const char* src, const char* dst) {
	FILE*       src_file;
    FILE*       dst_file;
	struct stat src_stats;
    struct stat dst_stats;
    size_t      copied;
    size_t      resume_chunk = 0;

    /* Open the src and dst files */
    src_file        = open_src_file(src);
    if(!src_file) {
        return -1;
    }

    dst_file        = open_dst_file(dst);
    if(!dst_file) {
        fclose(src_file);
        return -1;
    }

    /* Get the size of the src file */
    if(get_file_stats(src, &src_stats) != 0) {
        fclose(src_file);
        fclose(dst_file);
        return -1;
    }

    if(stat(dst, &dst_stats) == 0) {
        copy_existing_action_t action = prompt_user_action(dst);

        switch(action) {
            case COPY_SKIP:
                fclose(src_file);
                fclose(dst_file);
                return 0;
            case COPY_OVERWRITE:
                fclose(dst_file);
                dst_file = fopen(dst, "w+b");
                if(!dst_file) {
                    fclose(src_file);
                    return -1;
                }
                resume_chunk = 0;
                break;
            case COPY_RESUME:
                resume_chunk = determine_resume_chunk(src_file, dst_file, dst);
                break;
        }
    }

    reposition_streams_for_resume(src_file, dst_file, resume_chunk);

    /* Process all the chunks until everything is copied */
    copied = process_chunks(src_file, dst_file, &src_stats);
    copied += resume_chunk * CHUNK_SIZE;
    if(copied > (size_t) src_stats.st_size) {
        copied = src_stats.st_size;
    }

    /* Close the files */
    fclose(src_file);
    fclose(dst_file);

    return copied;
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

