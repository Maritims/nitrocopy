#include "fletcher32.h"

int fletcher32_debug = 0;

unsigned long fletcher32(const unsigned short* data, size_t words_remaining) {
    unsigned long   sum1        = 0xFFFF;
    unsigned long   sum2        = 0xFFFF;
    size_t          word_index  = 0;

    if(fletcher32_debug) {
        printf("Fletcher-32: Processing %lu words\n", words_remaining);
        printf("%-6s %-8s %-10s %-10s\n", "Index", "Word", "Sum1", "Sum2");
        printf("--------------------------------------\n");
    }

    if(words_remaining == 0 || data == NULL) {
        return (sum2 << 16) | sum1;
    }

    while(words_remaining > 0) {
        size_t words_to_process = words_remaining > 360 ? 360 : words_remaining;
        words_remaining -= words_to_process;

        while(words_to_process > 0) {
            unsigned short word = *data++;

            sum1 += word;
            sum2 += sum1;
            word_index++;
            words_to_process--;

            if(fletcher32_debug) {
                printf("%-6lu 0x%04X   0x%08lX 0x%08lX\n", word_index - 1, word, sum1, sum2);
            }
        }

        if(fletcher32_debug) {
            printf("Folding sums to 16 bits:\n");
            printf("  Before: sum1=0x%08lX, sum2=0x%08lX\n", sum1, sum2);
        }

        sum1 = (sum1 & 0xFFFF) + (sum1 >> 16);
        sum2 = (sum2 & 0xFFFF) + (sum2 >> 16);

        if(fletcher32_debug) {
            printf("  After:  sum1=0x%08lX, sum2=0x%08lX\n", sum1, sum2);
            printf("--------------------------------------\n");
        }
    }

    sum1 = (sum1 & 0xFFFF) + (sum1 >> 16);
    sum2 = (sum2 & 0xFFFF) + (sum2 >> 16);

    if(fletcher32_debug) {
        printf("Final checksum: 0x%08lX\n", (sum2 << 16) | sum1);
    }

    return (sum2 << 16) | sum1;
}
