#ifndef LOGGING_H
#define LOGGING_H

#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARN    3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_DEBUG   5

void log_fatal(const char* format, ...);

void log_error(const char* format, ...);

void log_warn(const char* format, ...);

void log_info(const char* format, ...);

void log_debug(const char* format, ...);

#endif
