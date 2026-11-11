#include "gdt/gdt.h"

static Tss tss;

GdtEntry create_gdt_entry(uint8_t accessByte, uint32_t limit, uint8_t flags) {
    GdtEntry entry{};

    entry.accessByte = accessByte;
    entry.lowerLimit = static_cast<uint16_t>(limit);
    entry.higherLimit_flags = static_cast<uint8_t>(((limit >> 16) & 0xF) | (flags & 0xF0));

    return entry;
}

TssDescriptor create_tss_descriptor(uint64_t tssAddress) {
    TssDescriptor descriptor{};

    descriptor.accessByte = TSS_DESCRIPTOR;
    descriptor.lowerBase1 = static_cast<uint16_t>(tssAddress);
    descriptor.lowerBase2 = static_cast<uint8_t>(tssAddress >> 16);
    descriptor.upperBase1 = static_cast<uint8_t>(tssAddress >> 24);
    descriptor.upperBase2 = static_cast<uint32_t>(tssAddress >> 32);
    descriptor.lowerLimit = 103;
    descriptor.higherLimit_flags = static_cast<uint8_t>(((103 >> 16) & 0xF));

    return descriptor;
}

void initialize_tss(uint64_t kernelStackTop) {
    tss = {};
}