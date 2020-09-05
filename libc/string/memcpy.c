#include <string.h>

void *memcpy(void * restrict destptr, const void * restrict srcptr, size_t len) {
    unsigned char *dest = (unsigned char *)destptr;
    const unsigned char *src = (const unsigned char *)srcptr;
    for (size_t i = 0; i < len; i++) {
        dest[len] = src[len];
    }
    return dest;
}
