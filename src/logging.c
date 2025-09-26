#include "logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

static void log_msg(int level, const char* format, va_list args) {
    time_t      rawtime;
    struct tm*  info;
    char        timestamp[72];
    const char* level_str;

    if(level > LOG_LEVEL) {
        return;
    }

    time(&rawtime);
    info = localtime(&rawtime);

    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
        info->tm_year + 1900,
        info->tm_mon + 1,
        info->tm_mday,
        info->tm_hour,
        info->tm_min,
        info->tm_sec
    );

    switch(level) {
        case LOG_LEVEL_FATAL:
            level_str = "FATAL";
        break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
        break;
        case LOG_LEVEL_WARN:
            level_str = "WARN";
        break;
        case LOG_LEVEL_INFO:
            level_str = "INFO";
        break;
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
        break;
    }

    printf("[%s] [%5s] - ", timestamp, level_str);
    vprintf(format, args);
}

void log_fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_msg(LOG_LEVEL_FATAL, format, args);
    va_end(args);
}

void log_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_msg(LOG_LEVEL_ERROR, format, args);
    va_end(args);
}

void log_warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_msg(LOG_LEVEL_WARN, format, args);
    va_end(args);
}

void log_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_msg(LOG_LEVEL_INFO, format, args);
    va_end(args);
}

void log_debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_msg(LOG_LEVEL_DEBUG, format, args);
    va_end(args);
}

