#ifndef WORD16_H
#define WORD16_H

#include <stdio.h>

/**
 * word16_count(): Count the number of 16 bit words in the input string.
 * Add 1 to the length then divide by 2 to ensure that any leftover byte get its own word.
 * Without adding 1 an odd-length string would lose its last byte.
 *
 * @input: The string to count number of words in.
 *
 * @return The number of 16-bit words in the input string.
 */
size_t word16_count(const char* input);

/**
 * word16_create(): Create 16 bit words in little endian order, low byte first and high byte second.
 *
 * @input: The string to create words from.
 * @out_words: The pointer to where the words should be stored.
 *
 * @return THe number of 16-bit words in the input string.
 */
size_t word16_create(const char* input, unsigned short* out_words);

#endif
