#include "tables/gdt.h"

static Tss tss;
alignas(16) static Gdt gdt;
static GdtPointer gdtPointer;

GdtEntry create_gdt_entry(uint8_t accessByte, uint32_t limit, uint8_t flags) {
    GdtEntry entry{};

    entry.accessByte = accessByte;
    entry.lowerLimit = static_cast<uint16_t>(limit);
    entry.higherLimit_flags = static_cast<uint8_t>(((limit >> 16) & 0xF) | (flags & 0xF0));

    return entry;
}

TssDescriptor create_tss_descriptor(uint64_t tssAddress) {
    TssDescriptor descriptor{};

    descriptor.accessByte = TSS_ACCESS;
    descriptor.lowerBase1 = static_cast<uint16_t>(tssAddress);
    descriptor.lowerBase2 = static_cast<uint8_t>(tssAddress >> 16);
    descriptor.upperBase1 = static_cast<uint8_t>(tssAddress >> 24);
    descriptor.upperBase2 = static_cast<uint32_t>(tssAddress >> 32);
    descriptor.lowerLimit = 103; // sizeof(tss) - 1 = 103
    descriptor.higherLimit_flags = static_cast<uint8_t>(((103 >> 16) & 0xF));

    return descriptor;
}

void initialize_tss(uint64_t kernelStackTop, uint64_t doubleFaultStackTop) {
    tss = {};
    
    tss.rsp[0] = kernelStackTop;
    tss.ist[0] = doubleFaultStackTop;
    tss.ioMapBase = sizeof(Tss);
}

void gdt_init(uint64_t kernelStackTop, uint64_t doubleFaultStackTop) {
    asm volatile("cli");

    gdt.null = {};
    gdt.kernelCode = create_gdt_entry(KERNEL_CODE_ACCESS, 0xFFFFF, CODE_FLAG_ACCESS);
    gdt.kernelData = create_gdt_entry(KERNEL_DATA_ACCESS, 0xFFFFF, DATA_FLAG_ACCESS);
    gdt.userData = create_gdt_entry(USER_DATA_ACCESS, 0xFFFFF, DATA_FLAG_ACCESS);
    gdt.userCode = create_gdt_entry(USER_CODE_ACCESS, 0xFFFFF, CODE_FLAG_ACCESS);

    initialize_tss(kernelStackTop, doubleFaultStackTop);

    gdt.tssDescriptor = create_tss_descriptor(reinterpret_cast<uint64_t>(&tss));

    gdtPointer.size = sizeof(gdt) - 1;
    gdtPointer.offset = reinterpret_cast<uint64_t>(&gdt);

    asm volatile("lgdt %0" : : "m"(gdtPointer) : "memory");

    asm volatile("pushq %0\n leaq 1f(%%rip), %%rax\n pushq %%rax\n lretq\n 1:\n" : : "i"(KERNEL_CODE_SELECTOR) : "rax", "memory");

    asm volatile("mov %w0, %%ds" : : "r"(KERNEL_DATA_SELECTOR));
    asm volatile("mov %w0, %%es" : : "r"(KERNEL_DATA_SELECTOR));
    asm volatile("mov %w0, %%ss" : : "r"(KERNEL_DATA_SELECTOR));
    asm volatile("mov %w0, %%fs" : : "r"(KERNEL_DATA_SELECTOR));
    asm volatile("mov %w0, %%gs" : : "r"(KERNEL_DATA_SELECTOR));

    asm volatile("ltr %w0" : : "r"(TSS_DESCRIPTOR_SELECTOR) : "memory");
}