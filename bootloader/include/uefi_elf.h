#ifndef _UEFI_ELF_H
#define _UEFI_ELF_H

typedef unsigned char byte_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define ELF_MAGIC0	0x7F
#define ELF_MAGIC1	0x45 // 'E'
#define ELF_MAGIC2  0x4c // 'L'
#define ELF_MAGIC3  0x46 // 'F'

#define ELF_MAGIC_OK(x) ((x)[0]==ELF_MAGIC0 && (x)[1]==ELF_MAGIC1 \
				&& (x)[2]==ELF_MAGIC2 && (x)[3]==ELF_MAGIC3)

typedef struct Elf64_header_t {
    struct {
        union {
            uint32_t magic_num;
            char magic_chars[4];
        } magic_num;
        byte_t class;
        byte_t data;
        byte_t version;
        byte_t os_abi;
        byte_t abi_ver;
        byte_t unused_0[7];
    } ident;
    uint16_t obj_type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry_addr;
    uint64_t prog_header_table_offset;
    uint64_t section_table_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t prog_header_entry_size;
    uint16_t prog_header_entry_num;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_num;
    uint16_t shstrndx;
} Elf64_header;

typedef struct Elf64_program_table_entry_t {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesize;
    uint64_t memsize;
    uint64_t align;
} Elf64_program_table_entry;

typedef struct Elf64_section_table_entry_t {
    uint32_t name_offset;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t align;
    uint64_t entry_size;
} Elf64_section_table_entry;

#endif