#include <efi/boot-services.h>
#include <efi/runtime-services.h>
#include <efi/system-table.h>
#include <efi/types.h>
#include <efi/protocol/simple-file-system.h>

#include <stdbool.h>

#include "memcpy.h"
#include "uefi_print.h"
#include "uefi_file.h"
#include "uefi_elf.h"

#undef ERR
#define ERR(x) if(EFI_ERROR((x))) do { print_hex64(u"Error: 0x", (x)); return (x); } while (0)

static unsigned char memmap_buf[32768];
static char memmap_file_text[32768];

static efi_system_table *st;

/*
size_t strlen(const char *str) {
    size_t l = 0;
    while (str[l] != '\0') l++;
    return l;
}*/

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

efi_status efi_main(efi_handle handle __attribute__((unused)), efi_system_table *st_in) {
    st = st_in;
    print_init(st);

    efi_status status;
    efi_input_key key;

    status = st->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    ERR(status);

    status = st->ConOut->SetMode(st->ConOut, 2);
    ERR(status);

    status = st->ConOut->ClearScreen(st->ConOut);
    ERR(status);

    status = st->ConOut->OutputString(st->ConOut, u"Hello, UEFI World!\n\r");
    ERR(status);

/*
    for (int32_t i = 0; i < st->ConOut->Mode->MaxMode; i++) {
        print_hex64(u"Mode ", i);
        int width;
        int height;
        status = st->ConOut->QueryMode(st->ConOut, i, &width, &height);
        ERR(status);
        print_hex64(u"Width ", width);
        print_hex64(u"Height ", height);
    }*/


    long long tst = 0;
    __asm__ __volatile__
    (
        "mov $0x11234567890, %0;"
        : "=r" (tst) :
    );
    if (tst == 0x11234567890) {
    } else {
        return EFI_UNSUPPORTED;
    }

    status = st->ConIn->Reset(st->ConIn, false);
    ERR(status);

    efi_file_protocol *config_handle;
    size_t config_size;
    size_t config_pages;
    efi_physical_addr config_buf;

    status = file_open(st->BootServices, &config_handle, u"boot/resourcesboot.cfg", EFI_FILE_MODE_READ);
    ERR(status);
    status = file_read(st->BootServices, config_handle, &config_buf, &config_size, &config_pages);
    ERR(status);
    status = file_close(config_handle);
    ERR(status);

    status = println_char((char*)config_buf, config_size);
    ERR(status);

    // @TODO: Use boot.cfg to configure launch of the kernel

    status = st->BootServices->FreePages(config_buf, config_pages);
    ERR(status);

    efi_file_protocol *kernel_handle;
    size_t kernel_size;
    size_t kernel_pages;
    efi_physical_addr kernel_addr;

    status = file_open(st->BootServices, &kernel_handle, u"test.out", EFI_FILE_MODE_READ);
    ERR(status);
    status = file_read(st->BootServices, kernel_handle, &kernel_addr, &kernel_size, &kernel_pages);
    ERR(status);
    status = file_close(kernel_handle);
    ERR(status);

    char *kernel_buf = (char*)kernel_addr;
    Elf64_header *elf_header = (Elf64_header *)kernel_buf;

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

    /*Elf64_section_table_entry *section_table_entries = 
            (Elf64_section_table_entry*)((uint64_t)elf_header + elf_header->section_table_offset);*/

    efi_memory_descriptor *mem_map = 
        (efi_memory_descriptor *)(memmap_buf + 2 * sizeof(uint64_t));
    size_t map_size = sizeof(memmap_buf) - 2 * sizeof(uint64_t);
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
    size_t num_descriptors = map_size / desc_size;
    memcpy(memmap_buf, &desc_size, sizeof(uint64_t));
    memcpy(memmap_buf + sizeof(uint64_t), &num_descriptors, sizeof(uint64_t));

    print_hex64(u"Map size: 0x", map_size);
    print(u"\n\r");
    print_hex64(u"Descriptor size: 0x", desc_size);
    print(u"\n\r");
    print_hex64(u"sizeof(efi_memory_descriptor): 0x", sizeof(efi_memory_descriptor));
    print(u"\n\r");
    print_hex64(u"Desc ver: 0x", desc_ver);
    print(u"\n\r");

    {
        efi_memory_descriptor *descriptor = mem_map + 3 * desc_size;
        descriptor->NumberOfPages = 0x400;
        descriptor->PhysicalStart = 0xFFC00000;
        descriptor->VirtualStart = 0xFF800000;
        descriptor->Type = ((efi_memory_descriptor*)(mem_map + desc_size))->Type;
        descriptor->Attribute = ((efi_memory_descriptor*)(mem_map + desc_size))->Attribute;
    }

    size_t first_invalid = 0;
    for (size_t i = 0; i < 0x17; i++) {
        efi_memory_descriptor *descriptor = mem_map + i * desc_size;
        if (descriptor->Type >= EfiMaxMemoryType) {
            first_invalid = 0;
            descriptor->Type = 0;
            descriptor->PhysicalStart = 0;
            descriptor->VirtualStart = 0;
            descriptor->NumberOfPages = 0;
            descriptor->Attribute = 0;
        } else {
            print_hex64(u"desc 0x", i);
            print_hex64(u" phys 0x", descriptor->PhysicalStart);
            print_hex64(u" virt 0x", descriptor->VirtualStart);
            print_hex64(u", pages 0x", descriptor->NumberOfPages);
            print(u"\n\r");
        }
    }
    
    st->BootServices->ExitBootServices(handle, map_key);

    size_t mem_map_size = desc_size * num_descriptors;
    st->RuntimeServices->SetVirtualAddressMap(mem_map_size, ((size_t*)memmap_buf)[0], desc_ver, mem_map);

/*
    char16_t a[1];
    a[0] = 'W' | (15 | 8 << 4) << 8;
    print(a);*/

    //st->BootServices->SignalEvent()

    //st->BootServices->ExitBootServices(handle, map_key);

    void *segment_pages[2];
    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        uint64_t page_addr = program_table_entries[i].paddr - (program_table_entries[i].paddr) % 0x1000;
/*
        efi_memory_descriptor *descriptor = mem_map + num_descriptors * desc_size;
        num_descriptors++;
        descriptor->NumberOfPages = kernel_pages;
        descriptor->PhysicalStart = kernel_addr;
        descriptor->VirtualStart = page_addr;
        descriptor->Type = EfiLoaderCode;
        descriptor->Attribute = 0;

        print_hex64(u"descriptor ", i);
        print_hex64(u"    descriptor->NumberOfPages = 0x", descriptor->NumberOfPages);
        print_hex64(u"    descriptor->PhysicalStart = 0x", descriptor->PhysicalStart);
        print_hex64(u"    descriptor->VirtualStart = 0x", descriptor->VirtualStart);
        print_hex64(u"    descriptor->Type = 0x", descriptor->Type);
        print_hex64(u"    descriptor->Attribute = 0x", descriptor->Attribute);

        program_table_entries[i].type;
*/
/*
        //@TODO: Allocate enough memory to hold all the data
        status = st->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, 1, &page_addr);
        ERR(status);

        st->BootServices->CopyMem((void*)page_addr, 
                                (void*)kernel_buf, 
                                kernel_size);

        segment_pages[i] = (void*)page_addr;*/
    }
    //st->RuntimeServices.


    //status = print_hex64(u"elf_header->entry_addr: 0x", elf_header->entry_addr);
    ERR(status);

    typedef uint64_t(*kmain_t)();
    kmain_t kmain = (kmain_t)elf_header->entry_addr;
    //hexdump(kmain, 32);
    //uint64_t kernel_return_code = kmain();
    
    //status = print(u"kmain() result: 0x");
    ERR(status);
    //status = print_hex64(u"", kernel_return_code);
    ERR(status);

    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        status = st->BootServices->FreePages((efi_physical_addr)segment_pages[i], 1);
    }

    status = st->BootServices->FreePages(kernel_buf, kernel_pages);
    ERR(status);

    status = st->ConOut->OutputString(st->ConOut, u"Application End!\n\r");
    ERR(status);

    while ((status = st->ConIn->ReadKeyStroke(st->ConIn, &key)) == EFI_NOT_READY);
    ERR(status);
    return EFI_SUCCESS;
}
