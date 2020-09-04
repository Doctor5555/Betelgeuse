#include <stdint.h>

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[20] = u"0000000000000000\n\r\0";

uint16_t * main(uint64_t a, uint64_t b) {
    /**** Convert UEFI calling convention to AMD64 ****/
    __asm__ __volatile__
    (
        "movq %%rcx, %0;" : "=r" (a) :
    );
    __asm__ __volatile__
    (
        "movq %%rdx, %0;" : "=r" (b) :
    );

    /**** Do the stuff ****/
    int64_t val = a + b; // BETELGEUSE

    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }

    return hex64outstr;
}
