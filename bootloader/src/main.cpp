extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

#include "boot_info.h"

#define EI_NIDENT   16
#define ELFCLASS64  2
#define EM_X86_64   62
#define ET_EXEC     2
#define PT_LOAD     1

struct Elf64_Ehdr {
    UINT8  e_ident[EI_NIDENT];  // 0x7f 'E' 'L' 'F', class, endianness, ...
    UINT16 e_type;              // ET_EXEC for your kernel
    UINT16 e_machine;           // EM_X86_64
    UINT32 e_version;
    UINT64 e_entry;             // address to jump to
    UINT64 e_phoff;             // file offset of the program header table
    UINT64 e_shoff;             // file offset of the section header table
    UINT32 e_flags;
    UINT16 e_ehsize;
    UINT16 e_phentsize;         // size of one program header
    UINT16 e_phnum;             // number of program headers
    UINT16 e_shentsize;
    UINT16 e_shnum;
    UINT16 e_shstrndx;
};

struct Elf64_Phdr {
    UINT32 p_type;              // PT_LOAD for loadable segments
    UINT32 p_flags;             // R/W/X permissions
    UINT64 p_offset;            // where the segment's bytes are in the file
    UINT64 p_vaddr;             // virtual address
    UINT64 p_paddr;             // physical (load) address
    UINT64 p_filesz;            // bytes stored in the file
    UINT64 p_memsz;             // bytes in memory (>= filesz, the rest is .bss)
    UINT64 p_align;
};

typedef void (__attribute__((sysv_abi)) *KernelEntry)(BootInfo*);

extern "C" EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    EFI_LOADED_IMAGE_PROTOCOL* loadedImage = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = NULL;
    EFI_FILE_PROTOCOL* rootDir = NULL;
    EFI_FILE_PROTOCOL* kernel = NULL;
    
    Print((const CHAR16*)u"Attempting to get Loaded Image!\r\n");

    EFI_STATUS status = uefi_call_wrapper(BS->HandleProtocol, 3,
        ImageHandle, &LoadedImageProtocol, (void**)&loadedImage);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to get Loaded Image! Error: %r\n", status);
        while (1) asm volatile("hlt");
    }
    Print((const CHAR16*)u"Successfully got Loaded Image!!\r\n");
    Print((const CHAR16*)u"Attempting to get the Simple File Protocal!\r\n");

    status = uefi_call_wrapper(BS->HandleProtocol, 3, loadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to get Simple File Protocal! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }
    Print((const CHAR16*)u"Successfully got Simple File Protocal!!\r\n");
    Print((const CHAR16*)u"Attempting to mount root volume!\r\n");

    status = uefi_call_wrapper(fileSystem->OpenVolume, 2, fileSystem, &rootDir);   
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to mount root volume! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    } 
    Print((const CHAR16*)u"Successfully mounted root volume!\r\n");
    Print((const CHAR16*)u"Attempting to open kernel.elf!\r\n");

    CHAR16* filePath = (CHAR16*)u"\\boot\\kernel.elf";
    status = uefi_call_wrapper(rootDir->Open, 5, rootDir, &kernel, filePath, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to open kernel.elf! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }
    Print((const CHAR16*)u"Successfully opened kernel.elf!\r\n");
    Print((const CHAR16*)u"Getting Kernel info!\r\n");

    EFI_FILE_INFO* fileInfo = LibFileInfo(kernel);
    if (!fileInfo) {
        Print((const CHAR16*)u"Failed to get kernel info! Error: EFI_BUFFER_TOO_SMALL\r\n");
        while (1)
            asm volatile("hlt");
    }
    Print((const CHAR16*)u"Allocating memory for kernel!\r\n");
    UINTN fileSize = fileInfo->FileSize;
    UINTN numPages = (fileSize + 4095) / 4096;
    FreePool(fileInfo);

    EFI_PHYSICAL_ADDRESS targetAddress = 0; 

    void* fileBuffer = NULL;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, numPages, &targetAddress);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to allocate memory for kernel! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }
    Print((const CHAR16*)u"Successfully allocated memory for kernel!\r\n");
    Print((const CHAR16*)u"Attempting to read kernel.elf!\r\n");
    
    status = uefi_call_wrapper(kernel->Read, 3, kernel, &fileSize, (void*)targetAddress);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to read kernel.elf! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }

    uefi_call_wrapper(kernel->Close, 1, kernel);
    uefi_call_wrapper(rootDir->Close, 1, rootDir);

    Elf64_Ehdr* eh = (Elf64_Ehdr*)targetAddress;
    if (eh->e_ident[0] != 0x7f || eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L'  || eh->e_ident[3] != 'F') {
        Print((const CHAR16*)u"Not an ELF file!\r\n");
        while (1) asm volatile("hlt");
    }

    

    Elf64_Phdr* ph = (Elf64_Phdr*)((UINT8*)targetAddress + eh->e_phoff);

    UINT64 lo = ~0ULL, hi = 0;
    for (UINT16 i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;
        if (ph[i].p_paddr < lo) lo = ph[i].p_paddr;
        if (ph[i].p_paddr + ph[i].p_memsz > hi) hi = ph[i].p_paddr + ph[i].p_memsz;
    }
    lo &= ~0xFFFULL;
    hi = (hi + 0xFFF) & ~0xFFFULL;

    EFI_PHYSICAL_ADDRESS addr = lo;
    status = uefi_call_wrapper(BS->AllocatePages, 4,
        AllocateAddress, EfiLoaderCode, (hi - lo) / 0x1000, &addr);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Kernel alloc at %lx failed: %r\r\n", lo, status);
        while (1) asm volatile("hlt");
    }

    for (UINT16 i = 0; i < eh->e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) continue;
        CopyMem((VOID*)ph[i].p_paddr, (UINT8*)targetAddress + ph[i].p_offset, ph[i].p_filesz);
        SetMem((VOID*)(ph[i].p_paddr + ph[i].p_filesz), ph[i].p_memsz - ph[i].p_filesz, 0);
    }


    UINT64 entryAddr = eh->e_entry;
    Print((const CHAR16*)u"entry=%lx bytes: %02x %02x %02x\r\n", entryAddr,
          ((UINT8*)entryAddr)[0], ((UINT8*)entryAddr)[1], ((UINT8*)entryAddr)[2]);


    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
    
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to locate GOP\n");
        return status;
    }

    uint64_t bufferSize = gop->Mode->Info->HorizontalResolution * gop->Mode->Info->VerticalResolution * sizeof(unsigned int);
    uint64_t pagesNeeded = (bufferSize + 4095) / 4096;

    EFI_PHYSICAL_ADDRESS backBufferAddress;
    status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, pagesNeeded, &backBufferAddress);

    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to allocate back buffer memory pages\n");
        return status;
    }

    UINTN MapSize = 0;
    UINTN BufSize = 0;
    EFI_MEMORY_DESCRIPTOR* MemMap = NULL;
    UINTN MapKey;
    UINTN DescriptorSize;
    UINT32 DescriptorVersion;

    uefi_call_wrapper(BS->GetMemoryMap, 5, &MapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    MapSize += 16 * DescriptorSize;
    BufSize = MapSize;
    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, MapSize, (void**)&MemMap);
    
    status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to get memory map! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }
    
    bool exited = false;
    for (int attempt = 0; attempt < 5; attempt++) {
        MapSize = BufSize;
        status = uefi_call_wrapper(BS->GetMemoryMap, 5, &MapSize, MemMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (EFI_ERROR(status)) return status;

        status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, MapKey);
        if (!EFI_ERROR(status)) {
            exited = true;
            break;
        }
        
    }
    if (!exited)
        return status;


    KernelEntry start_kernel = (KernelEntry)entryAddr;
    
    BootInfo bInfo = {};
    bInfo.frontBuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    bInfo.screenWidth = gop->Mode->Info->HorizontalResolution;
    bInfo.screenHeight = gop->Mode->Info->VerticalResolution;
    bInfo.pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;
    
    start_kernel(&bInfo);
    for (;;) asm volatile("hlt");

    return status;
}