#ifndef KEYBOARD_H_GUARD
#define KEYBOARD_H_GUARD

#include "types.h"

typedef struct kb_result {
    char c;
    u8 modifiers;
    u8 scancode;
    u8 do_not_print;
} kb_result;

void kb_read(kb_result *result);

#endif /* KEYBOARD_H_GUARD */