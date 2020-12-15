#include <string.h>

void *memset(void *dest_pointer, const int byte, size_t len) {
    unsigned char *dest = (unsigned char *)dest_pointer;
    for (size_t i = 0; i < len; i++) {
        dest[i] = byte;
    }
    return dest;
}
