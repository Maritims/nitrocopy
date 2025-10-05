#ifndef FLETCHER_32_H
#define FLETCHER_32_H

#include <stdio.h>

extern int fletcher32_debug;

/**
 * Fletcher-32 operates on 16-bit words, so the data is expected to be of unsigned short.
 * In C89 unsigned long is guaranteed to be at least 32 bits. An unsigned int can be 16-bit, so we can't trust that it's any larger.
 */
unsigned long fletcher32(const unsigned short* words, size_t number_of_words);

#endif
