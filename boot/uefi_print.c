#include "uefi_print.h"

#include "common.h"

static efi_system_table *st;

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[19] = u"0000000000000000\n\r";

static const uint16_t CHAR_BUF_MAX_LEN = 1024;
static uint16_t println_char_wchar_buf[1024];

efi_status print_init(efi_system_table *st_in) {
    st = st_in;
    return EFI_SUCCESS;
}

efi_status print_hex64(char16_t *msg, size_t val) {
    efi_status status;
    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    status = st->ConOut->OutputString(st->ConOut, msg);
    ERR(status);
    status = st->ConOut->OutputString(st->ConOut, hex64outstr);
    return status;
}

efi_status print(char16_t *msg) {
    efi_status status;
    status = st->ConOut->OutputString(st->ConOut, msg);
    return status;
}

efi_status println(char16_t *msg) {
    efi_status status;
    status = st->ConOut->OutputString(st->ConOut, msg);
    ERR(status);
    status = st->ConOut->OutputString(st->ConOut, u"\n\r");
    return status;
}

efi_status println_char(char *msg, size_t length) {
    for (size_t i = 0; i < length; i++) {
        println_char_wchar_buf[i] = msg[i];
    }
    println_char_wchar_buf[length] = '\0';

    efi_status status = st->ConOut->OutputString(st->ConOut, println_char_wchar_buf);
    ERR(status);
    status = st->ConOut->OutputString(st->ConOut, u"\n\r");
    return status;
}

efi_status hexdump(char *msg, size_t length) {
    efi_status status;
    size_t val = 0;
    size_t i = 0;
    for (i; i < length; i += 8) {
        val = msg[i+0] << 7 * 8 | 
              msg[i+1] << 6 * 8 | 
              msg[i+2] << 5 * 8 | 
              msg[i+3] << 4 * 8 | 
              msg[i+4] << 3 * 8 | 
              msg[i+5] << 2 * 8 | 
              msg[i+6] << 1 * 8 | 
              msg[i+7] << 0 * 8;
        status = print_hex64(L"", val);
        ERR(status);
    }
    return status;
}