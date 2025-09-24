#ifndef NITRO_LOGGING_H
#define NITRO_LOGGING_H

#define NITRO_LOG_LEVEL_FATAL   1
#define NITRO_LOG_LEVEL_ERROR   2
#define NITRO_LOG_LEVEL_WARN    3
#define NITRO_LOG_LEVEL_INFO    4
#define NITRO_LOG_LEVEL_DEBUG   5

void nitro_log_fatal(const char* format, ...);

void nitro_log_error(const char* format, ...);

void nitro_log_warn(const char* format, ...);

void nitro_log_info(const char* format, ...);

void nitro_log_debug(const char* format, ...);

#endif
