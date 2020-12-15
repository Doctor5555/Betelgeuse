#include <string.h>

void *memmove(void *dest_pointer, const void *source_pointer, size_t len) {
    unsigned char *dest = (unsigned char *)dest_pointer;
    const unsigned char *source = (const unsigned char *)source_pointer;
    if (dest < source) {
        for (size_t i = 0; i < len; i++)
            dest[i] = source[i];
    } else {
        for (size_t i = len; i != 0; i--)
            dest[i-1] = source[i-1];
    }
    return dest_pointer;
}