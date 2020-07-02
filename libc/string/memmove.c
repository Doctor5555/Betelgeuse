#include <string.h>

void *memmove(void *destptr, const void *srcptr, size_t len) {
    unsigned char *dest = (unsigned char *)destptr;
    const unsigned char *src = (const unsigned char *)srcptr;
    if (dest < src) {
        for (size_t i = 0; i < len; i++)
            dest[i] = src[i];
    } else {
        for (size_t i = len; i != 0; i--)
            dest[i-1] = src[i-1];
    }
    return destptr;
}