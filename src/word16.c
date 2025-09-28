#include "word16.h"
#include <string.h>

size_t word16_count(const char* input) {
    size_t input_length = strlen(input);
    return (input_length + 1) / 2;
}

size_t word16_create(const char* input, unsigned short* out_words) {
    size_t input_length = strlen(input);
    size_t word_count   = word16_count(input);
    size_t word_index;

    for(word_index = 0; word_index < word_count; word_index++) {
        /* The high byte is the second character in a pair, so by multiplying the index we arrive at the character following the one at the current index */
        unsigned char high_byte = (unsigned char) input[word_index * 2];
        unsigned char low_byte  = 0;
        if(word_index * 2 + 1 < input_length) {
            low_byte = (unsigned char) input[word_index * 2 + 1]; 
        }

        out_words[word_index] = ((unsigned short) low_byte << 8) | high_byte;
    }

    return word_index;
}
