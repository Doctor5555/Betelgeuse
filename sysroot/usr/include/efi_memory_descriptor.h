// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <stdint.h>

typedef uint64_t efi_physical_addr;
typedef uint64_t efi_virtual_addr;

typedef struct {
  uint32_t Type;
  efi_physical_addr PhysicalStart;
  efi_virtual_addr VirtualStart;
  uint64_t NumberOfPages;
  uint64_t Attribute;
} efi_memory_descriptor;