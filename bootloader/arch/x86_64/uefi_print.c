#include "uefi_print.h"

#include "common.h"
#include "uefi_file.h"

static efi_system_table *st;

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[19] = u"0000000000000000";
static uint16_t hex8outstr[19] = u"00";

static uint16_t println_char_wchar_buf[1024];

efi_status echo_to_logfile(char16_t *msg, size_t len) {
    efi_status status;
    efi_file_protocol *file_handle;
    efi_physical_addr buf;
    size_t pages;
    size_t size;
    status = file_open(st->BootServices, &file_handle, u"b.txt", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE);
    ERR(status);
    status = file_read(st->BootServices, file_handle, &buf, &size, &pages);
    ERR(status);
    char16_t *char_buf = buf;
    for (size_t i = 0; i < len; i++) {
        char_buf[size] = msg[i];
        size++;
    }

    status = file_write(file_handle, size, buf);
    ERR(status);
    status = file_close(file_handle);
    ERR(status);
    return status;
}

size_t str16len(char16_t *msg) {
    size_t len = 0;
    while (msg[len] != 0) {
        len++;
    }
    return len;
}
size_t strlen(char *msg) {
    size_t len = 0;
    while (msg[len] != 0) {
        len++;
    }
    return len;
}

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

efi_status print_hex8(char16_t *msg, char val) {
    efi_status status;
    for (int8_t i = 1; i >= 0; i--) {
        hex8outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    status = st->ConOut->OutputString(st->ConOut, msg);
    ERR(status);
    status = st->ConOut->OutputString(st->ConOut, hex8outstr);
    return status;
}

efi_status print(char16_t *msg) {
    efi_status status;
    status = st->ConOut->OutputString(st->ConOut, msg);
    //echo_to_logfile(msg, str16len(msg));
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
    size_t i = 0;
    for (; i < length; i += 8) {
        status = print_hex8(L"", msg[i]);
        ERR(status);
        status = print_hex8(L" ", msg[i+1]);
        ERR(status);
        status = print_hex8(L" ", msg[i+2]);
        ERR(status);
        status = print_hex8(L" ", msg[i+3]);
        ERR(status);
        status = print_hex8(L" ", msg[i+4]);
        ERR(status);
        status = print_hex8(L" ", msg[i+5]);
        ERR(status);
        status = print_hex8(L" ", msg[i+6]);
        ERR(status);
        status = print_hex8(L" ", msg[i+7]);
        ERR(status);
        st->ConOut->OutputString(st->ConOut, u"\n\r");
        ERR(status);
    }
    return status;
}
