#include "boot_info.h"

extern "C" [[noreturn]] void kernel_main(BootInfo* binfo) {
	unsigned long long total_pixels = binfo->screenHeight * binfo->pixelsPerScanline;
    for (unsigned long long i = 0; i < total_pixels; i++) {
        binfo->frontBuffer[i] = 0xFFF;
    }
	 for (;;) asm volatile("hlt");
}