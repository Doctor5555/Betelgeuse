#include <stdint.h>

uint64_t main() {
    /**** Convert UEFI calling convention to AMD64 ****/
    /*
    __asm__ __volatile__
    (
        "movq %%rcx, %0;" : "=r" (a) :
    );
    __asm__ __volatile__
    (
        "movq %%rdx, %0;" : "=r" (b) :
    );*/

    /**** Do the stuff ****/
    /*
    int64_t val = a + b; // BETELGEUSE

    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
*/

    uint16_t *vga_text_buf = 0xB8000;
    vga_text_buf[0] = 'W' | (15 | 8 << 4) << 8;

    return 0xBE2E76E43E;
}
