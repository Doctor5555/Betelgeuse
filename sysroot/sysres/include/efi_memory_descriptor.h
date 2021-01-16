// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef _EFI_MEMORY_DESCRIPTOR_H
#define _EFI_MEMORY_DESCRIPTOR_H

#include <stdint.h>

typedef uint64_t efi_physical_addr;
typedef uint64_t efi_virtual_addr;

typedef enum {
  EfiReservedMemoryType,
  EfiLoaderCode,
  EfiLoaderData,
  EfiBootServicesCode,
  EfiBootServicesData,
  EfiRuntimeServicesCode,
  EfiRuntimeServicesData,
  EfiConventionalMemory,
  EfiUnusableMemory,
  EfiACPIReclaimMemory,
  EfiACPIMemoryNVS,
  EfiMemoryMappedIO,
  EfiMemoryMappedIOPortSpace,
  EfiPalCode,
  EfiPersistentMemory,
  EfiMaxMemoryType
} efi_memory_type;

typedef struct {
  efi_memory_type type;
  efi_physical_addr physical_start;
  efi_virtual_addr virtual_start;
  uint64_t number_of_pages;
  uint64_t attribute;
} efi_memory_descriptor;

#endif /* _EFI_MEMORY_DESCRIPTOR_H */