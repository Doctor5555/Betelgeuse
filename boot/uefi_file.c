#include "uefi_file.h"

#include "uefi_print.h"

#include <efi/protocol/simple-file-system.h>

efi_status file_open(efi_boot_services *bs, efi_file_protocol **file_handle, const char16_t *filename, uint64_t open_mode) {
    efi_status status;

    size_t handle_count = 0;
    efi_handle *handles = NULL;
    status = bs->LocateHandleBuffer(ByProtocol, 
            &SimpleFileSystemProtocol,
            NULL,
            &handle_count,
            &handles);
    ERR(status);

    size_t i = 0;

    for (i; i < handle_count; i++)
    {
        efi_simple_file_system_protocol *fs = NULL;

        status = bs->HandleProtocol(
            handles[i],
            &SimpleFileSystemProtocol,
            (void**)&fs);
        ERR(status);

        efi_file_protocol *root = NULL;
        status = fs->OpenVolume(fs, &root);
        ERR(status);

        status = root->Open(
            root,
            file_handle,
            filename, 
            open_mode,
            EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
        if (!EFI_ERROR(status)) {
            status = root->Close(root);
            ERR(status);
            break;
        }
        status = root->Close(root);
        ERR(status);
    }

    if (i == handle_count)
        return EFI_NOT_FOUND;
    return EFI_SUCCESS;
}

efi_status file_read(efi_boot_services *bs, efi_file_protocol *file_handle, void **file_contents, size_t *file_size) {
    efi_status status;

    efi_file_info *finfo = NULL;
    size_t finfosize = sizeof(efi_file_info);
    status = file_handle->GetInfo(file_handle, &FileInfoGuid, &finfosize, finfo);
    if (status == EFI_BUFFER_TOO_SMALL) {
        status = bs->AllocatePool(
            EfiLoaderData, 
            finfosize, 
            &finfo);
        ERR(status);
        status = file_handle->GetInfo(file_handle, &FileInfoGuid, &finfosize, finfo);
        ERR(status);
    } else {
        ERR(status);
    }

    *file_size = finfo->FileSize;
    status = bs->AllocatePool(
            EfiLoaderData, 
            *file_size, 
            file_contents);
        ERR(status);

    file_handle->Read(file_handle, file_size, *file_contents);
    ERR(status);

    return EFI_SUCCESS;
}

efi_status file_close(efi_file_protocol  *file_handle) {
    file_handle->Close(file_handle);
}

efi_status file_write(efi_file_protocol *file_handle, size_t len, char *buf) {
    file_handle->Write(file_handle, len, buf);
}