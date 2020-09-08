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

    for (; i < handle_count; i++)
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
            0);
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

efi_status file_open_ex(efi_boot_services *bs, efi_file_protocol *root_handle, efi_file_protocol **file_handle, const char16_t *filename, uint64_t open_mode) {
    efi_status status;
    status = root_handle->Open(root_handle, file_handle, filename, open_mode, 0);
    return status;
}

efi_status file_read(efi_boot_services *bs, efi_file_protocol *file_handle, efi_physical_addr *file_contents, size_t *file_size, size_t *pages) {
    efi_status status;

    uint8_t need_free_finfo = 0;
    efi_file_info *finfo = NULL;
    size_t finfosize = sizeof(efi_file_info);
    status = file_handle->GetInfo(file_handle, &FileInfoGuid, &finfosize, finfo);
    if (status == EFI_BUFFER_TOO_SMALL) {
        status = bs->AllocatePool(
            EfiLoaderData, 
            finfosize, 
            (void**)&finfo);
        ERR(status);
        status = file_handle->GetInfo(file_handle, &FileInfoGuid, &finfosize, finfo);
        ERR(status);
        need_free_finfo = 1;
    } else {
        ERR(status);
    }

    *file_size = finfo->FileSize;
    *pages = *file_size / 0x1000 + 1;
    status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData, *pages, file_contents);
    /*status = bs->AllocatePool(
            EfiLoaderData, 
            *file_size,
            file_contents);*/
    ERR(status);

    status = file_handle->Read(file_handle, file_size, *file_contents);
    ERR(status);

    if (need_free_finfo != 0) {
        bs->FreePool(finfo);
    }

    return EFI_SUCCESS;
}

efi_status file_close(efi_file_protocol  *file_handle) {
    return file_handle->Close(file_handle);
}

efi_status file_write(efi_file_protocol *file_handle, size_t len, char *buf) {
    return file_handle->Write(file_handle, &len, buf);
}