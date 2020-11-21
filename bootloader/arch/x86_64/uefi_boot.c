/*
uefi_boot.c
Main file in the bootloader. This file is responsible for preparing and launching the kernel.
Pipeline:

Initial setup
Test 64 bit support
Open config file
Parse config file @TODO
Open and read font file
Display boot menu @TODO @LOW
Open selected kernel
Retrieve and save memory map
Exit boot services
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
#include "uefi_mem.h"
#include "bootloader/boot_table.h"
#include "uefi_tty.h"

#undef ERR
#define ERR(x) if(EFI_ERROR((x))) do { print_hex64(u"Error: 0x", (x)); print(u"\n\r"); goto end; } while (0)

static unsigned char memmap_buf[0x8000];
static unsigned char psf_buf[0x8000];
static struct boot_table boot_table;

static unsigned char scratch_stack[0x1000];
static unsigned char *stack_ptr = scratch_stack;

static efi_system_table *st;

/*
void __attribute__((noreturn)) 
jump_to_kmain(uint64_t entry_addr, void *boot_table) {
    __asm__ __volatile__ (
        "movq %0, %%rax;"
        "movq %1, %%rcx;"
        "jmp *%%rax": : "r" (entry_addr), "r" (boot_table)
    );
}*/

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

    /* @TODO: Use boot.cfg to configure launch of the kernel */

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

    status = print_hex64(u"Framebuffer base: 0x", boot_table.graphics_mode.framebuffer_base);
    ERR(status);
    print(u"\n\r");
    status = print_hex64(u"Kernel entry address: 0x", elf_header->entry_addr);
    ERR(status);
    print(u"\n\r");

    boot_table.kernel_start_ptr = (unsigned long long)elf_header->entry_addr;

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

    boot_table.mem_table_ptr = memmap_buf;
    boot_table.mem_desc_size = desc_size;
    boot_table.mem_desc_count = desc_count;
    boot_table.font_ptr = psf_buf;

    /* Exit boot services and init virtual mapping*/
    status = st->BootServices->ExitBootServices(handle, map_key);
    ERR(status);

    init_virtual_mapping(mem_map, desc_count, desc_size);
    get_mapping_ptrs_and_count(&boot_table.pml4_ptr, &boot_table.next_available_mapping_page, &boot_table.available_mapping_page_count);
    
    /* Parse the kernel header and map it to the correct memory location */
    Elf64_program_table_entry *program_table_entries = 
            (Elf64_program_table_entry*)((uint64_t)elf_header + elf_header->prog_header_table_offset);
    /*Elf64_section_table_entry *section_table_entries = 
            (Elf64_section_table_entry*)((uint64_t)elf_header + elf_header->section_table_offset);*/

    uint64_t highest_page_mapped = 0;
    for (uint64_t i = 0; i < elf_header->prog_header_entry_num; i++) {
        uint64_t segment_size;
        if (i == elf_header->prog_header_entry_num - 1) {
            segment_size = kernel_size - program_table_entries[i].offset;
        } else {
            segment_size = program_table_entries[i + 1].offset - program_table_entries[i].offset;
        }

        virtual_addr segment_base_addr      = program_table_entries[i].vaddr;
        virtual_addr segment_page_base_addr = segment_base_addr - segment_base_addr % 0x1000;
        virtual_addr segment_max_addr       = segment_base_addr + segment_size;
        virtual_addr segment_page_max_addr  = segment_max_addr - segment_max_addr % 0x1000;
        uint64_t pages                      = 1 + (segment_page_max_addr - segment_page_base_addr) / 0x1000;

        physical_addr segment_phys_addr = kernel_buf + program_table_entries[i].offset;

        if (segment_base_addr + (pages << 12) > highest_page_mapped) {
            highest_page_mapped = segment_base_addr + (pages << 12);
        }
        status = map_pages(segment_phys_addr, segment_base_addr, pages);

        if(EFI_ERROR(status)) {
            terminal_init(&boot_table);
            terminal_writestring("Mapping error: ");
            terminal_print_hex64(status);
            terminal_writestring("\n\r");
            return status;
        }
    }
    //terminal_writestring("Mapped kernel, jumping in!\n\r");

    /* Call the kernel */
    typedef uint64_t(*kmain_t)(struct boot_table*);
    kmain_t kmain = elf_header->entry_addr;
    uint64_t kernel_return = kmain(&boot_table);

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
