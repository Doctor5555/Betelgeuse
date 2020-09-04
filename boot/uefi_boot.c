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
    } else {
        status = st->ConOut->OutputString(st->ConOut, u"Long mode enabled!\n\r");
        ERR(status);
        return EFI_UNSUPPORTED;
    }

    status = st->ConIn->Reset(st->ConIn, false);
    ERR(status);

    efi_file_protocol *config_handle;
    size_t config_size;
    char *config_buf;

    status = file_open(st->BootServices, &config_handle, u"boot.cfg", EFI_FILE_MODE_READ);
    ERR(status);
    status = file_read(st->BootServices, config_handle, &config_buf, &config_size);
    ERR(status);
    status = file_close(config_handle);
    ERR(status);

    status = println_char(config_buf, config_size);
    ERR(status);
    status = st->BootServices->FreePool(config_buf);
    ERR(status);

    efi_file_protocol *kernel_handle;
    size_t kernel_size;
    char *kernel_buf;
    status = file_open(st->BootServices, &kernel_handle, u"test.out", EFI_FILE_MODE_READ);
    ERR(status);

    status = file_read(st->BootServices, kernel_handle, &kernel_buf, &kernel_size);
    ERR(status);

    status = file_close(kernel_handle);
    ERR(status);

    Elf64_header *elf_header = kernel_buf;

    char* elf_magic = "0ELF";
    elf_magic[0] = 0x7F;
    int equal = 1;
    for (uint8_t i = 0; i < 4; i++) {
        if (elf_magic[i] != kernel_buf[i]) {
            equal = 0;
            break;
        }
    }

    if (equal) {
    } else {
        status = print(u"Invalid ELF file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->ident.class == 0x02) {
    } else {
        status = print(u"Invalid ELF64 file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->machine == 0x3E) {
    } else {
        status = print(u"Invalid ELF64 file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    Elf64_program_table_entry *program_table_entries = 
            (Elf64_program_table_entry*)((uint64_t)elf_header + elf_header->prog_header_table_offset);

    Elf64_section_table_entry *section_table_entries = 
            (Elf64_section_table_entry*)((uint64_t)elf_header + elf_header->section_table_offset);

    void *segment_pages[2];
    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        uint64_t page_addr = program_table_entries[i].paddr - (program_table_entries[i].paddr) % 0x1000;

        //@TODO: Allocate enough memory to hold all the data
        status = st->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, 1, &page_addr);
        ERR(status);

        st->BootServices->CopyMem((void*)page_addr, 
                                (void*)kernel_buf, 
                                kernel_size);

        segment_pages[i] = page_addr;
    }

    status = print_hex64(u"elf_header->entry_addr: 0x", elf_header->entry_addr);
    ERR(status);

    typedef uint64_t(*test_main)(uint64_t a, uint64_t b);

    test_main fn = elf_header->entry_addr;
    uint64_t a = 0xBE2E76E43E;
    uint64_t b = 2;
    uint64_t str = fn(a, b);

    status = print(u"fn() result: 0x");
    ERR(status);
    status = print_hex64(u"", str);
    ERR(status);

    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        status = st->BootServices->FreePages(segment_pages[i], 1);
    }

    status = st->BootServices->FreePool(kernel_buf);
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

    memcpy(memmap_buf, &desc_size, sizeof(uint64_t));

    status = st->ConOut->OutputString(st->ConOut, u"Application End!\n\r");
    ERR(status);

end:
    while ((status = st->ConIn->ReadKeyStroke(st->ConIn, &key)) == EFI_NOT_READY);
    ERR(status);
    return EFI_SUCCESS;
}
