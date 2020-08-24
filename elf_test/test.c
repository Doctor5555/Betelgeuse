#include <stdint.h>

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[20] = u"0000000000000000\n\r\0";

uint16_t *main(uint64_t a) {
    /*int64_t val = 0xBE2E76E43E; // BETELGEUSE

    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }*/

    __asm__ __volatile__
    (
        "mov %%rbx, %0;"
        : "=r" (a) :
    );

    //a = *(uint64_t*)a;

    hex64outstr[0] = (a >> 48) & 0xFF;
    hex64outstr[1] = (a >> 32) & 0xFF;
    hex64outstr[2] = (a >> 16) & 0xFF;
    hex64outstr[3] = (a >> 00) & 0xFF;
    return hex64outstr;
}