#include "memcpy.h"

#include <stddef.h>

void memcpy(void *dest_ptr, void *src_ptr, size_t size) {
    char *dest = dest_ptr;
    char *src = src_ptr;
    for (size_t i = 0; i < size; i++) {
        dest[i] = src[i];
    }
}
