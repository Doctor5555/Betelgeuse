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

    status = print_hex64(u"handle_count: 0x", handle_count);
    ERR(status);

    status = print_hex64(u"handles: 0x", handles);
    ERR(status);

    status = print_hex64(u"handles[0]: 0x", handles[0]);
    ERR(status);
    
    size_t i = 0;

    for (i; i < handle_count; i++)
    {
        status = print_hex64(u"i: 0x", i);
        ERR(status);
        efi_simple_file_system_protocol *fs = NULL;

        status = bs->HandleProtocol(
            handles[i],
            &SimpleFileSystemProtocol,
            (void**)&fs);
        ERR(status);

        status = print_hex64(u"fs: 0x", fs);
        ERR(status);

        efi_file_protocol *root = NULL;
        status = fs->OpenVolume(fs, &root);
        ERR(status);

        status = print_hex64(u"root: 0x", root);
        ERR(status);

        /*
        efi_file_system_info *fsinfo = NULL;
        size_t fsinfosize = sizeof(efi_file_system_info);
        status = root->GetInfo(root, &FileSystemInfoGuid, &fsinfosize, fsinfo);
        if (status == EFI_BUFFER_TOO_SMALL) {
            status = bs->AllocatePool(
                EfiLoaderData, 
                fsinfosize, 
                &fsinfo);
            ERR(status);
            status = root->GetInfo(root, &FileSystemInfoGuid, &fsinfosize, fsinfo);
            ERR(status);
        } else {
            ERR(status);
        }

        status = print_hex64(u"fsinfo->Size: 0x", fsinfo->Size);
        ERR(status);

        status = print(u"fsinfo->VolumeLabel: ");
        ERR(status);
        status = println(fsinfo->VolumeLabel);
        ERR(status);
        */

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

        status = print_hex64(u"Failed to open file: 0x", status);
        ERR(status);
        status = root->Close(root);
        ERR(status);
        status = print_hex64(u"index2: 0x", i);
        ERR(status);

        if (i >= handle_count) {
            status = print_hex64(u"index2: 0x", i);
            ERR(status);
        }
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

    status = print_hex64(u"finfo->Size: 0x", finfo->Size);
    ERR(status);
    status = print_hex64(u"finfo->FileSize: 0x", finfo->FileSize);
    ERR(status);
    status = print_hex64(u"finfo->PhysicalSize: 0x", finfo->PhysicalSize);
    ERR(status);

    *file_size = finfo->FileSize;
    status = bs->AllocatePool(
            EfiLoaderData, 
            *file_size, 
            file_contents);
        ERR(status);

    file_handle->Read(file_handle, file_size, *file_contents);
    ERR(status);

    status = print_hex64(u"buf: 0x", file_contents);
    ERR(status);

    status = print_hex64(u"buf[0]: 0x", ((char*)file_contents)[0]);
    ERR(status);

    status = print_hex64(u"file_size: 0x", *file_size);
    ERR(status);

    return EFI_SUCCESS;
}

efi_status file_close(efi_file_protocol  *file_handle) {
    file_handle->Close(file_handle);
}

efi_status file_write(efi_file_protocol *file_handle, size_t len, char *buf) {
    file_handle->Write(file_handle, len, buf);
}