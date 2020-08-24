#include <efi/boot-services.h>
#include <efi/runtime-services.h>
#include <efi/system-table.h>
#include <efi/types.h>
#include <efi/protocol/simple-file-system.h>

#include <stdbool.h>

#include "memcpy.h"
#include "uefi_print.h"
//#include "uefi_file.h"

#define ERR(x) if(EFI_ERROR((x))) do { print_hex64(u"Error: 0x", (x)); return (x); } while (0)

static unsigned char memmap_buf[32768];

static efi_system_table *st;

size_t strlen(const char *str) {
    size_t l = 0;
    while (str[l] != '\0') l++;
    return l;
}

/*
#define EI_NIDENT        16
typedef struct {
    unsigned char  e_ident[EI_NIDENT];
    Elf32_Half     e_type;
    Elf32_Half     e_machine;
    Elf32_Word     e_version;
    Elf32_Addr     e_entry;
    Elf32_Off      e_phoff;
    Elf32_Off      e_shoff;
    Elf32_Word     e_flags;
    Elf32_Half     e_ehsize;
    Elf32_Half     e_phentsize;
    Elf32_Half     e_phnum;
    Elf32_Half     e_shentsize;
    Elf32_Half     e_shnum;
    Elf32_Half     e_shstrndx;
} Elf32_Ehdr;
*/

typedef unsigned char byte_t;

typedef struct {
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

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesize;
    uint64_t memsize;
    uint64_t align;
} Elf64_program_table_entry;

typedef struct {
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

uint16_t *testfn(uint64_t a) {
    return a;
}

static char hexchars[17] = "0123456789ABCDEF";
static uint16_t hex64outstr[19] = u"0000000000000000\n\r";

size_t add_and_print(efi_simple_text_output_protocol *ConOut, int a, int b) {
    size_t val = a + b;

    efi_status status;
    for (int8_t i = 15; i >= 0; i--) {
        hex64outstr[i] = hexchars[val & 0b1111];
        val = val >> 4;
    }
    status = ConOut->OutputString(ConOut, u"Sum: ");
    ERR(status);
    status = ConOut->OutputString(ConOut, hex64outstr);
    return status;
}

efi_status efi_main(efi_handle handle __attribute__((unused)), efi_system_table *st_in,
        void* image, size_t sz, void* ramdisk, size_t rsz) {

    st = st_in;
    print_init(st);

    efi_status status;
    efi_input_key key;

    status = st->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    ERR(status);

    status = st->ConOut->ClearScreen(st->ConOut);
    ERR(status);

    status = st->ConOut->OutputString(st->ConOut, u"Hello, UEFI World!\n\r");
    ERR(status);

    long long tst = 0;
    __asm__ __volatile__
    (
        "mov $0x11234567890, %0;"
        : "=r" (tst) :
    );
    if (tst == 0x11234567890) {
        status = st->ConOut->OutputString(st->ConOut, u"Long mode enabled!\n\r");
        ERR(status);
    }

    print_hex64(u"tst: 0x", tst);

    status = print_hex64(u"ramdisk: 0x", ramdisk);
    ERR(status);
    status = print_hex64(u"sz: 0x", sz);
    ERR(status);
    status = print_hex64(u"rsz: 0x", rsz);
    ERR(status);
    status = print_hex64(u"handle: 0x", handle);
    ERR(status);

    status = st->ConIn->Reset(st->ConIn, false);
    ERR(status);

    efi_file_protocol *test_handle;
    size_t test_size;
    char *test_buf;
    status = file_open(st->BootServices, &test_handle, u"b.txt", EFI_FILE_MODE_READ);
    ERR(status);

    status = print_hex64(u"test_size: 0x", test_size);
    ERR(status);
    status = print_hex64(u"test_buf: 0x", test_buf);
    ERR(status);
    status = print_hex64(u"test_handle: 0x", test_handle);
    ERR(status);
    status = print_hex64(u"*test_handle: 0x", *(size_t*)test_handle);
    ERR(status);

    status = file_read(st->BootServices, test_handle, &test_buf, &test_size);
    ERR(status);

    status = file_close(test_handle);
    ERR(status);

    status = println_char(test_buf, test_size);
    ERR(status);

    status = st->BootServices->FreePool(test_buf);
    ERR(status);

    efi_file_protocol *b_handle;
    size_t b_size;
    char *b_buf;
    status = file_open(st->BootServices, &b_handle, u"a.txt", EFI_FILE_MODE_READ);
    ERR(status);

    status = print_hex64(u"b_size: 0x", b_size);
    ERR(status);
    status = print_hex64(u"b_buf: 0x", b_buf);
    ERR(status);
    status = print_hex64(u"b_handle: 0x", b_handle);
    ERR(status);
    status = print_hex64(u"*b_handle: 0x", *(size_t*)b_handle);
    ERR(status);

    status = file_read(st->BootServices, b_handle, &b_buf, &b_size);
    ERR(status);

    status = file_close(b_handle);
    ERR(status);

    status = println_char(b_buf, b_size);
    ERR(status);
    status = st->BootServices->FreePool(b_buf);
    ERR(status);

    efi_file_protocol *c_handle;
    size_t c_size;
    char *c_buf;
    status = file_open(st->BootServices, &c_handle, u"c.cfg", EFI_FILE_MODE_READ);
    ERR(status);

    status = file_read(st->BootServices, c_handle, &c_buf, &c_size);
    ERR(status);

    status = file_close(c_handle);
    ERR(status);

    status = println_char(c_buf, c_size);
    ERR(status);
    status = st->BootServices->FreePool(c_buf);
    ERR(status);

    status = file_open(st->BootServices, &c_handle, u"test.out", EFI_FILE_MODE_READ);
    ERR(status);

    status = file_read(st->BootServices, c_handle, &c_buf, &c_size);
    ERR(status);

    status = file_close(c_handle);
    ERR(status);

    Elf64_header *elf_header = c_buf;
    
    status = print_hex64(u"elf_header->ident.magic_num: ", elf_header->ident.magic_num.magic_num);
    ERR(status);
    status = print_hex64(u"elf_header->ident.data: ", elf_header->ident.data);
    ERR(status);

/*
    status = println_char(c_buf + 0, 1);
    ERR(status);
    status = println_char(c_buf + 1, 1);
    ERR(status);
    status = println_char(c_buf + 2, 1);
    ERR(status);
    status = println_char(c_buf + 3, 1);
    ERR(status);*/

    char *elf_magic = "0ELF";
    elf_magic[0] = 0x7F;
    int equal = 1;
    for (uint8_t i = 0; i < 4; i++) {
        if (elf_magic[i] != c_buf[i]) {
            equal = 0;
            break;
        }
    }

    if (equal) {
        status = print(u"Valid ELF file!\n\r");
        ERR(status);
    } else {
        status = print(u"Invalid ELF file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->ident.class == 0x02) {
        status = print(u"Valid ELF64 file!\n\r");
        ERR(status);
    } else {
        status = print(u"Invalid ELF64 file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->machine == 0x3E) {
        status = print(u"Valid ELF64 file!\n\r");
        ERR(status);
    } else {
        status = print(u"Invalid ELF64 file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    status = print_hex64(u"elf_header->prog_header_table_offset: ", elf_header->prog_header_table_offset);
    ERR(status);
    status = print_hex64(u"elf_header->prog_header_entry_num: ", elf_header->prog_header_entry_num);
    ERR(status);

    status = print_hex64(u"elf_header->section_table_offset: ", elf_header->section_table_offset);
    ERR(status);
    status = print_hex64(u"elf_header->section_header_entry_num: ", elf_header->section_header_entry_num);
    ERR(status);

    Elf64_program_table_entry *program_table_entries = (Elf64_program_table_entry*)((uint64_t)elf_header + elf_header->prog_header_table_offset);
    Elf64_section_table_entry *section_table_entries = (Elf64_section_table_entry*)((uint64_t)elf_header + elf_header->section_table_offset);

    status = print_hex64(u"program_table_entries[0].type: ", program_table_entries[0].type);
    ERR(status);
    status = print_hex64(u"program_table_entries[1].type: ", program_table_entries[1].type);
    ERR(status);
    status = print_hex64(u"section_table_entries[0].type: ", section_table_entries[0].type);
    ERR(status);

    //status = hexdump(c_buf, 32);
    ERR(status);

    //status = print(u"Hexdump c_buf + 0x40, 32:\n\r");
    ERR(status);
    //status = hexdump(c_buf + 0x40, 32);
    ERR(status);

    void *segment_pages[2];

    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        uint64_t page_addr = program_table_entries[i].paddr - (program_table_entries[i].paddr) % 0x1000;

        status = print_hex64(u"program_table_entries[i].paddr: ", program_table_entries[i].paddr);
        ERR(status);

        //@TODO: Allocate enough memory to hold all the data
        status = st->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, 1, &page_addr);
        ERR(status);
        status = print_hex64(u"page_addr: 0x", page_addr);
        ERR(status);
        status = print_hex64(u"program_table_entries[i].filesize: 0x", program_table_entries[i].filesize);
        ERR(status);
        status = print_hex64(u"program_table_entries[i].offset: 0x", program_table_entries[i].offset);
        ERR(status);

        st->BootServices->CopyMem((void*)page_addr, 
                                (void*)c_buf, 
                                c_size);

        segment_pages[i] = page_addr;
    }

/*
    for (size_t i = 0; i < elf_header->section_header_entry_num; i++) {
        uint64_t page_addr = section_table_entries[i].addr;
        if (page_addr == 0)
            continue;

        status = print_hex64(u"section_table_entries[i].addr: ", page_addr);
        ERR(status);
        status = print_hex64(u"section_table_entries[i].size: 0x", section_table_entries[i].size);
        ERR(status);
        status = print_hex64(u"section_table_entries[i].offset: 0x", section_table_entries[i].offset);
        ERR(status);

        st->BootServices->CopyMem((void*)page_addr, 
                                (void*)(c_buf + section_table_entries[i].offset), 
                                section_table_entries[i].size);
    }*/

    status = print_hex64(u"elf_header->entry_addr: 0x", elf_header->entry_addr);
    ERR(status);

    typedef uint16_t*(*test_main)(uint64_t a);

    test_main fn = elf_header->entry_addr;
    uint64_t a = 0xBE2E76E43E;
    uint16_t b = 2;
    uint16_t c = 3;
    uint16_t d = 4;
    uint16_t e = 5;
    uint16_t f = 6;
    uint16_t g = 7;
    uint16_t h = 8;
    uint16_t i = 9;
    uint16_t j = 10;
    uint16_t k = 11;
    uint16_t l = 12;
    uint16_t m = 13;
    uint16_t n = 14;
    uint16_t o = 15;
    uint16_t p = 16;
    uint16_t *str = fn(a);
    //fn = testfn;
    testfn(a);

/*
    status = print(u"fn() result: 0x");
    ERR(status);
    status = print(str);
    ERR(status);*/

    status = print_hex64(u"fn() result: 0x", ((size_t*)str)[0]);
    ERR(status);

    status = print_hex64(u"segment_pages[0]: 0x", segment_pages[0]);
    ERR(status);
    status = print_hex64(u"segment_pages[1]: 0x", segment_pages[1]);
    ERR(status);

    //status = hexdump(add_and_print, 0x46);
    ERR(status);
    //status = print(u"entry_addr:\n\r");
    ERR(status);
    //status = hexdump(elf_header->entry_addr, 0x46);
    ERR(status);

    status = st->BootServices->FreePool(c_buf);
    ERR(status);

    efi_memory_descriptor *mem_map = 
        (efi_memory_descriptor *)(memmap_buf + sizeof(uint64_t));
    size_t map_size = sizeof(memmap_buf) - sizeof(uint64_t);
    size_t map_key = 0;
    size_t desc_size = 0;
    uint32_t desc_ver = 0;
    status = st->BootServices->GetMemoryMap(
        &map_size, mem_map, &map_key, &desc_size, &desc_ver);
    if (status != EFI_SUCCESS) {
        efi_status print_status = st->ConOut->OutputString(st->ConOut, u"Failed to retrieve memory map!\n\r");
        ERR(print_status);
        ERR(status);
    }

/*
    status = print_hex64(u"map_size: 0x", map_size);
    ERR(status);
    status = print_hex64(u"map_key: 0x", map_key);
    ERR(status);
    status = print_hex64(u"desc_size: 0x", desc_size);
    ERR(status);
    status = print_hex64(u"desc_ver: 0x", desc_ver);
    ERR(status);

    status = print_hex64(u"num descriptions: 0x", map_size / desc_size);
    ERR(status);

    status = print_hex64(u"mem_map->Type: 0x", (size_t)mem_map->Type);
    ERR(status);
    status = print_hex64(u"mem_map->NumberOfPages: 0x", (size_t)mem_map->NumberOfPages);
    ERR(status);*/


    memcpy(memmap_buf, &desc_size, sizeof(uint64_t));

    status = st->ConOut->OutputString(st->ConOut, u"Application End!\n\r");
    ERR(status);

end:
    while ((status = st->ConIn->ReadKeyStroke(st->ConIn, &key)) == EFI_NOT_READY);
    ERR(status);
    return EFI_SUCCESS;
}
