boot/uefi_boot.o: boot/uefi_boot.c efi/boot-services.h \
  efi/protocol/device-path.h efi/types.h efi/runtime-services.h \
  efi/system-table.h efi/protocol/simple-text-input.h \
  efi/protocol/simple-text-output.h efi/protocol/simple-file-system.h \
  efi/protocol/file.h boot/memcpy.h boot/uefi_print.h
