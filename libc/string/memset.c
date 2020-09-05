#include <string.h>

void *memset(void *destptr, const int byte, size_t len) {
    unsigned char *dest = (unsigned char *)destptr;
    for (size_t i = 0; i < len; i++) {
        dest[i] = byte;
    }
    return dest;
}
