typedef unsigned char bool;

#include <efi/protocol/simple-text-output.h>

#define ERR(x) if(EFI_ERROR((x))) return (x)

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[19] = u"0000000000000000\n\r";

size_t main(efi_simple_text_output_protocol *ConOut, int a, int b) {
    int val = a + b;

    efi_status status;
    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    status = ConOut->OutputString(ConOut, L"Sum: ");
    ERR(status);
    status = ConOut->OutputString(ConOut, hex64outstr);
    return status;
}