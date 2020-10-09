/*
uefi_boot.c
Main file in the bootloader. This file is responsible for preparing and launching the kernel.
Pipeline:

Initial setup
Test 64 bit support
Open config file
Parse config file @TODO
Open and read font file @TODO
Display boot meny @TODO @LOW
Open selected kernel
Retrieve and save memory map @TODO
Exit boot services @TODO
Launch selected kernel
*/

#include <efi/boot-services.h>
#include <efi/runtime-services.h>
#include <efi/system-table.h>
#include <efi/types.h>
#include <efi/protocol/simple-file-system.h>
#include <efi/protocol/graphics-output.h>
#include <psf.h>

#include <stdbool.h>

#include "memcpy.h"
#include "uefi_print.h"
#include "uefi_file.h"
#include "uefi_elf.h"
#include "boot_table.h"

#undef ERR
#define ERR(x) if(EFI_ERROR((x))) do { print_hex64(u"Error: 0x", (x)); print(u"\n\r"); goto end; } while (0)

static unsigned char memmap_buf[0x8000];
static unsigned char psf_buf[0x8000];
static struct boot_table boot_table;

static efi_system_table *st;

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
    if(EFI_ERROR((status))) do { print_hex64(u"Error: 0x", (status)); return status; } while (0);
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

    /* Check 64 bit mode */
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

    /* Open /boot/resources/boot.cfg */
    efi_file_protocol *config_handle = NULL;
    efi_file_protocol *boot_folder_handle = NULL;
    efi_file_protocol *resource_folder_handle = NULL;
    size_t config_size;
    size_t config_pages;
    efi_physical_addr config_buf;
    
    status = file_open(st->BootServices, &boot_folder_handle, u"boot", EFI_FILE_MODE_READ);
    ERR(status);
    print(u"Opened /boot\n\r");
    status = file_open_ex(st->BootServices, boot_folder_handle, &resource_folder_handle, u"resources", EFI_FILE_MODE_READ);
    ERR(status);
    print(u"Opened /boot/resources\n\r");
    status = file_open_ex(st->BootServices, resource_folder_handle, &config_handle, u"boot.cfg", EFI_FILE_MODE_READ);
    ERR(status);
    print(u"Opened /boot/resources/boot.cfg\n\r");
    status = file_read(st->BootServices, config_handle, &config_buf, &config_size, &config_pages);
    ERR(status);
    status = file_close(config_handle);
    ERR(status);

    status = println_char((char*)config_buf, config_size);
    ERR(status);

    // @TODO: Use boot.cfg to configure launch of the kernel

    status = st->BootServices->FreePages(config_buf, config_pages);
    ERR(status);

    /* Open the font from /boot/resources */
    efi_file_protocol *font_handle;
    size_t font_size;
    size_t font_pages;
    efi_physical_addr font_addr;

    status = file_open_ex(st->BootServices, resource_folder_handle, &font_handle, u"aply16.psf", EFI_FILE_MODE_READ);
    ERR(status);
    status = file_read(st->BootServices, font_handle, &font_addr, &font_size, &font_pages);
    ERR(status);
    status = file_close(font_handle);
    ERR(status);

    //unsigned char *font_buf = (unsigned char*)font_addr;
    struct psf2_header *font_psf_header = (struct psf2_header*)font_addr;

    /* Check it is a valid font */
    if (PSF2_MAGIC_OK(font_psf_header->magic)) {
    } else {
        status = print(u"Invalid PSF2 file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (font_pages > 0x8000 / 0x1000) {
        print(u"PSF file is too large (>32768 bytes)\n\r");
        return EFI_BUFFER_TOO_SMALL;
    }

    memcpy(psf_buf, (void *)font_addr, font_size);
    if (PSF2_MAGIC_OK(psf_buf)) {
    } else {
        status = print(u"Copy failed!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }
    status = st->BootServices->FreePages(font_addr, font_pages);
    ERR(status);

    print(u"Done font!\n\r");

    /* Open the kernel ELF binary */
    efi_file_protocol *kernel_handle;
    size_t kernel_size;
    size_t kernel_pages;
    efi_physical_addr kernel_addr;

    status = file_open_ex(st->BootServices, boot_folder_handle, &kernel_handle, u"betelgeuse.kernel", EFI_FILE_MODE_READ);
    ERR(status);
    status = file_read(st->BootServices, kernel_handle, &kernel_addr, &kernel_size, &kernel_pages);
    ERR(status);
    status = file_close(kernel_handle);
    ERR(status);

    char *kernel_buf = (char*)kernel_addr;
    Elf64_header *elf_header = (Elf64_header *)kernel_buf;

    /* Check the kernel is actually an ELF binary */
    //static const char* elf_magic = "\x7f""ELF";
    if (ELF_MAGIC_OK(elf_header->ident.magic_num.magic_chars)) {
    } else {
        status = print(u"Invalid ELF file, exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->ident.class == 0x02) {
    } else {
        status = print(u"Invalid ELF64 file (elf_header->ident.class), exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    if (elf_header->machine == 0x3E) {
    } else {
        status = print(u"Invalid ELF64 file (elf_header->machine), exiting!\n\r");
        ERR(status);
        return EFI_INVALID_PARAMETER;
    }

    print(u"Done kernel!\n\r");

    /* Parse the kernel header and load it to the correct memory location */
    Elf64_program_table_entry *program_table_entries = 
            (Elf64_program_table_entry*)((uint64_t)elf_header + elf_header->prog_header_table_offset);

    /*Elf64_section_table_entry *section_table_entries = 
            (Elf64_section_table_entry*)((uint64_t)elf_header + elf_header->section_table_offset);*/

    void *segment_pages[2];
    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        uint64_t page_addr = program_table_entries[i].paddr - (program_table_entries[i].paddr) % 0x1000;
        /*print_hex64(u"i: 0x", i);
        print_hex64(u", addr: 0x", page_addr);
        print_hex64(u", off: 0x", program_table_entries[i].offset);
        print_hex64(u", typ: 0x", program_table_entries[i].type);
        print(u"\n\r");*/
        // @TODO: Make the virtual addresses actually virtually addressed?
        // @TODO: Allocate enough memory to hold all the data
        
        status = st->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, 1, &page_addr);
        if (status != EFI_SUCCESS) {
            print_hex64(u"Error: 0x", (status));
            return status;
        }

        st->BootServices->CopyMem((void*)page_addr, 
                                (void*)(kernel_buf + program_table_entries[i].offset), 
                                kernel_size - program_table_entries[i].offset);

        segment_pages[i] = (void*)page_addr;
    }

    /* Locate the graphics output protocol*/
    size_t gop_handle_count = 0;
    efi_handle *gop_handles = NULL;
    status = st->BootServices->LocateHandleBuffer(ByProtocol, 
            &GraphicsOutputProtocol, 
            NULL, 
            &gop_handle_count, 
            &gop_handles);
    ERR(status);

    efi_graphics_output_protocol *graphics_output = NULL;
    status = st->BootServices->HandleProtocol(gop_handles[1], &GraphicsOutputProtocol, (void**)&graphics_output);
    ERR(status);
    print(u"Done find graphics protocol!\n\r");

    /* Search for and set graphics mode with resolution 1280x720 */
    size_t mode_size = 0;
    efi_graphics_output_mode_information *mode_info = NULL;
    size_t res_1280x720 = graphics_output->Mode->MaxMode + 1;
    for (size_t i = 0; i < graphics_output->Mode->MaxMode; i++) {
        status = graphics_output->QueryMode(graphics_output, i, &mode_size, &mode_info);
        if (status == EFI_BUFFER_TOO_SMALL) {
            status = st->BootServices->AllocatePool(EfiLoaderData, mode_size, (void **)&mode_info);
            ERR(status);
            status = graphics_output->QueryMode(graphics_output, i, &mode_size, &mode_info);
            ERR(status);
        } else {
            ERR(status);
        }
        
        if (mode_info->HorizontalResolution == 0x500) {
            print_hex64(u"Mode index: 0x", i);
            print_hex64(u", Width: 0x", mode_info->HorizontalResolution);
            print_hex64(u", Height: 0x", mode_info->VerticalResolution);
            print(u"\n\r");
            /*if ((mode_info->HorizontalResolution * 9) / 16 == mode_info->VerticalResolution)
                res_1280x720 = i;*/
            if (mode_info->VerticalResolution == 0x320)
                res_1280x720 = i;
            boot_table.graphics_mode.width = 0x500;
            boot_table.graphics_mode.height = 0x320;
            boot_table.graphics_mode.px_per_scan = mode_info->PixelsPerScanLine;
        }
        status = st->BootServices->FreePool(mode_info);
        ERR(status);
        mode_size = 0;
    }

    if (res_1280x720 == graphics_output->Mode->MaxMode + 1) {
        print(u"Error: 1280x720 resolution not supported!");
        return EFI_UNSUPPORTED;
    }
    
    graphics_output->SetMode(graphics_output, res_1280x720);
    status = st->ConOut->SetMode(st->ConOut, 2);
    ERR(status);
    status = st->ConOut->ClearScreen(st->ConOut);
    ERR(status);
    print(u"Changed resolution!\n\r");

    /* Set boot table framebuffer pointer */
    boot_table.graphics_mode.framebuffer_base = (void *)graphics_output->Mode->FrameBufferBase;
    boot_table.graphics_mode.framebuffer_size = graphics_output->Mode->FrameBufferSize;

    /*status = st->BootServices->FreePool(mode_info);
    ERR(status);*/

    status = print_hex64(u"Kernel entry address: 0x", elf_header->entry_addr);
    ERR(status);
    print(u"\n\r");

    boot_table.kernel_start_ptr = (unsigned long long)elf_header->entry_addr;
    status = print_hex64(u"Kernel entry address: 0x", boot_table.kernel_start_ptr);
    print(u"\n\r");
    if (boot_table.kernel_start_ptr == 0)
        goto end;
    
    //struct psf2_header* psf_header_ptr = ((struct psf2_header*)psf_buf);

    status = st->ConOut->ClearScreen(st->ConOut);
    ERR(status);

    /* Retrieve the memory map */
    efi_memory_descriptor *mem_map = 
        (efi_memory_descriptor *)memmap_buf;
    size_t map_size = sizeof(memmap_buf);
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
    size_t desc_count = map_size / desc_size;
    /*memcpy(memmap_buf, &desc_size, sizeof(uint64_t));
    memcpy(memmap_buf + sizeof(uint64_t), &desc_count, sizeof(uint64_t));*/

    boot_table.mem_table_ptr = memmap_buf;
    boot_table.mem_desc_size = desc_size;
    boot_table.mem_desc_count = desc_count;
    boot_table.font_ptr = psf_buf;

    /* Exit boot services and call the kernel */
    status = st->BootServices->ExitBootServices(handle, map_key);
    ERR(status);

    typedef uint64_t(*kmain_t)(struct boot_table *table);
    kmain_t kmain = (kmain_t)elf_header->entry_addr;
    //uint64_t kernel_return_code = 
    kmain(&boot_table);

    return EFI_SUCCESS;

    /* Pre-exit-boot-services abort */
end:
    status = file_close(boot_folder_handle);
    ERR(status);
    status = file_close(resource_folder_handle);
    ERR(status);

    status = st->ConOut->OutputString(st->ConOut, u"Application End!\n\r");
    ERR(status);
    status = st->ConIn->ReadKeyStroke(st->ConIn, &key);
    while (status == EFI_NOT_READY) {
        status = st->ConIn->ReadKeyStroke(st->ConIn, &key);
    }
    ERR(status);
    return EFI_SUCCESS;
}
    
    /*
    status = print(u"kmain() result: 0x");
    ERR(status);
    status = print_hex64(u"", kernel_return_code);
    ERR(status);
    print(u"\n\r");

    print_hex64(u"Expectd result: 0x", gop_handles);
    print(u"\n\r");*/

/*
    for (size_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        status = st->BootServices->FreePages((efi_physical_addr)segment_pages[i], 1);
    }

    status = st->BootServices->FreePages(kernel_addr, kernel_pages);
    ERR(status);

    print(u"Freed kernel!\n\r");*/

/*
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
    size_t desc_count = map_size / desc_size;
    memcpy(memmap_buf, &desc_size, sizeof(uint64_t));
    memcpy(memmap_buf + sizeof(uint64_t), &desc_count, sizeof(uint64_t));

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

    size_t mem_map_size = desc_size * desc_count;
    st->RuntimeServices->SetVirtualAddressMap(mem_map_size, ((size_t*)memmap_buf)[0], desc_ver, mem_map);
*/
/*
    char16_t a[1];
    a[0] = 'W' | (15 | 8 << 4) << 8;
    print(a);*/

    //st->BootServices->SignalEvent()

    //st->BootServices->ExitBootServices(handle, map_key);