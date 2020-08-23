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

    status = st->ConOut->OutputString(st->ConOut, L"Hello, UEFI World!\n\r");
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

    status = print_hex64(u"map_size: 0x", map_size);
    ERR(status);
    status = print_hex64(u"map_key: 0x", map_key);
    ERR(status);
    status = print_hex64(u"desc_size: 0x", desc_size);
    ERR(status);
    status = print_hex64(u"desc_ver: 0x", desc_ver);
    ERR(status);

    memcpy(memmap_buf, &desc_size, sizeof(uint64_t));

    efi_file_protocol *test_handle;
    size_t test_size;
    char *test_buf;
    status = file_open(st->BootServices, &test_handle, u"b.txt");
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
    status = file_open(st->BootServices, &b_handle, u"a.txt");
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
    status = file_open(st->BootServices, &c_handle, u"c.cfg");
    ERR(status);

    status = file_read(st->BootServices, c_handle, &c_buf, &c_size);
    ERR(status);

    status = file_close(c_handle);
    ERR(status);

    status = println_char(c_buf, c_size);
    ERR(status);
    status = st->BootServices->FreePool(c_buf);
    ERR(status);


    status = file_open(st->BootServices, &c_handle, u"test.out");
    ERR(status);

    status = file_read(st->BootServices, c_handle, &c_buf, &c_size);
    ERR(status);

    status = file_close(c_handle);
    ERR(status);

    status = hexdump(c_buf, 128);
    ERR(status);

    Elf64_header *elf_header = c_buf;
    
    status = print_hex64(L"elf_header->ident.magic_num: ", elf_header->ident.magic_num.magic_num);
    ERR(status);
    status = print_hex64(L"elf_header->ident.data: ", elf_header->ident.data);
    ERR(status);

    status = println_char(c_buf + 0, 1);
    ERR(status);
    status = println_char(c_buf + 1, 1);
    ERR(status);
    status = println_char(c_buf + 2, 1);
    ERR(status);
    status = println_char(c_buf + 3, 1);
    ERR(status);

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
        status = print(L"Valid ELF file!\n\r");
        ERR(status);
    }

    status = st->BootServices->FreePool(c_buf);
    ERR(status);

    /*
    status = println(u"Testing!");
    ERR(status);

    status = st->BootServices->FreePool(test_buf);
    ERR(status);
    status = st->BootServices->FreePool(b_buf);
    ERR(status);

    status = println(u"Testing!");
    ERR(status);
    
    goto end;
    */

    status = st->ConOut->OutputString(st->ConOut, L"Application End!\n\r");
    ERR(status);

end:
    while ((status = st->ConIn->ReadKeyStroke(st->ConIn, &key)) == EFI_NOT_READY);
    ERR(status);
    return EFI_SUCCESS;
}
