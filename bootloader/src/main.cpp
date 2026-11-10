extern "C" {
    #include <efi/efi.h>
    #include <efi/efilib.h>
}

typedef void (__attribute__((sysv_abi)) *KernelEntry)();

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
    FreePool(fileInfo);

    void* fileBuffer = NULL;
    status = uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, fileSize, &fileBuffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to allocate memory for kernel! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }
    Print((const CHAR16*)u"Successfully allocated memory for kernel!\r\n");
    Print((const CHAR16*)u"Attempting to read kernel.elf!\r\n");
    
    status = uefi_call_wrapper(kernel->Read, 3, kernel, &fileSize, fileBuffer);
    if (EFI_ERROR(status)) {
        Print((const CHAR16*)u"Failed to read kernel.elf! Error: %r\r\n", status);
        while (1)
            asm volatile("hlt");
    }

    uefi_call_wrapper(kernel->Close, 1, kernel);
    uefi_call_wrapper(rootDir->Close, 1, rootDir);

    KernelEntry start_kernel = (KernelEntry)fileBuffer;
    
    start_kernel();

    return status;
}