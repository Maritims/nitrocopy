#ifndef WORD16_H
#define WORD16_H

#include <stdio.h>

size_t word16_count(size_t length);

/**
 * word16_create(): Create 16 bit words in little endian order, low byte first and high byte second.
 *
 * @input: The string to create words from.
 * @out_words: The pointer to where the words should be stored.
 *
 * @return THe number of 16-bit words in the input string.
 */
size_t word16_create(const unsigned char* input, size_t length, unsigned short* output);

#endif
