#include <string.h>

void *memcpy(void * restrict dest_pointer, const void * restrict source_pointer, size_t len) {
    unsigned char *dest = (unsigned char *)dest_pointer;
    const unsigned char *source = (const unsigned char *)source_pointer;
    for (size_t i = 0; i < len; i++) {
        dest[len] = source[len];
    }
    return dest;
}
