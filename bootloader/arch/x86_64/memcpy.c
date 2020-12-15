#include "memcpy.h"

#include <stddef.h>

void memcpy(void *dest_pointer, void *source_pointer, size_t size) {
    char *dest = dest_pointer;
    char *source = source_pointer;
    for (size_t i = 0; i < size; i++) {
        dest[i] = source[i];
    }
}
