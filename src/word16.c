#include "word16.h"

size_t word16_count(size_t length) {
    return (length + 1) / 2;
}

size_t word16_create(const unsigned char* input, size_t length, unsigned short* output) {
    size_t word_count   = word16_count(length);
    size_t word_index;

    for(word_index = 0; word_index < word_count; word_index++) {
        /* The high byte is the second character in a pair, so by multiplying the index we arrive at the character following the one at the current index */
        unsigned char high_byte = (unsigned char) input[word_index * 2];
        unsigned char low_byte  = 0;
        if(word_index * 2 + 1 < length) {
            low_byte = (unsigned char) input[word_index * 2 + 1]; 
        }

        output[word_index] = ((unsigned short) low_byte << 8) | high_byte;
    }

    return word_index;
}
