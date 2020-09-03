#ifndef _UEFI_FILE_H
#define _UEFI_FILE_H

#include "common.h"

#include <efi/boot-services.h>
#include <efi/protocol/file.h>

efi_status file_open(in efi_boot_services *bs, out efi_file_protocol **file_handle, in const char16_t *filename, uint64_t open_mode);
efi_status file_read(in efi_boot_services *bs, in efi_file_protocol  *file_handle, out void **file_buffer, out size_t *test_size);
efi_status file_close(in efi_file_protocol  *file_handle);
efi_status file_write(efi_file_protocol *file_handle, size_t len, char *buf);

#endif /* _UEFI_FILE_H */