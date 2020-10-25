#ifndef _UEFI_PRINT_H
#define _UEFI_PRINT_H

#include <efi/system-table.h>

efi_status print_init(efi_system_table *st_in);

efi_status print_hex64(char16_t *msg, uint64_t val);

efi_status print(char16_t *msg);

efi_status println(char16_t *msg);

efi_status println_char(char *msg, size_t length);

efi_status hexdump(char *msg, size_t length);

#endif /* _UEFI_PRINT_H */