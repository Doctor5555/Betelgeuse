#include <efi/protocol/simple-file-system.h>
#include <efi/protocol/file.h>
#include <efi/protocol/device-path.h>
#include <efi/protocol/graphics-output.h>

efi_guid SimpleFileSystemProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
efi_guid FileInfoGuid = EFI_FILE_INFO_GUID;
efi_guid FileSystemInfoGuid = EFI_FILE_SYSTEM_INFO_GUID;
efi_guid DevicePathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
efi_guid GraphicsOutputProtocol = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;