#ifndef NITRO_DEBUG_H
#define NITRO_DEBUG_H

#include <stdio.h>
#include <stdarg.h>
#include "nitro_runtime.h"

#ifdef NITRO_DEBUG

#ifndef __func__
    #define __func__ "(unknown)"
#endif

static void nitro_debug(const char* file, int line, const char* func, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if(nitro_verbose) {
        fprintf(stderr, "[DEBUG] %s:%d:%s(): ", file, line, func);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }

    va_end(args);
}

#define NITRO_DEBUG_LOG(fmt, args) \
    nitro_debug(__FILE__, __LINE__, __func__, fmt, args)
#else
#define NITRO_DEBUG_LOG(fmt, args) ((void)0)
#endif

#endif
