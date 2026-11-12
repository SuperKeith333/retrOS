#pragma once

#include <cstddef>
#include <cstdint>

namespace AccessByte {
    constexpr uint8_t READ_WRITE = 1 << 1; // Set bit 1 to 1
    constexpr uint8_t EXECUTABLE = 1 << 3;
    constexpr uint8_t DESCRIPTOR_TYPE = 1 << 4;
    constexpr uint8_t PRESENT = 1 << 7;
    constexpr uint8_t RING0 = 0 << 5;
    constexpr uint8_t RING3 = 3 << 5;
    constexpr uint8_t AVAILABLE_TSS = 0x9;
}

namespace GdtFlags {
    constexpr uint8_t LONG_MODE = 1 << 5;
    constexpr uint8_t SIZE = 1 << 6;
    constexpr uint8_t GRANULARITY = 1 << 7;
}

namespace RplConstants {
    constexpr uint8_t RPL0 = 0;
    constexpr uint8_t RPL1 = 1;
    constexpr uint8_t RPL2 = 2;
    constexpr uint8_t RPL3 = 3;
}

constexpr uint8_t KERNEL_CODE_ACCESS = (AccessByte::PRESENT | AccessByte::DESCRIPTOR_TYPE | AccessByte::EXECUTABLE | AccessByte::READ_WRITE | AccessByte::RING0);
constexpr uint8_t KERNEL_DATA_ACCESS = (AccessByte::PRESENT | AccessByte::DESCRIPTOR_TYPE | AccessByte::READ_WRITE | AccessByte::RING0);
constexpr uint8_t USER_CODE_ACCESS = (AccessByte::PRESENT | AccessByte::DESCRIPTOR_TYPE | AccessByte::EXECUTABLE | AccessByte::READ_WRITE | AccessByte::RING3);
constexpr uint8_t USER_DATA_ACCESS = (AccessByte::PRESENT | AccessByte::DESCRIPTOR_TYPE | AccessByte::READ_WRITE | AccessByte::RING3);
constexpr uint8_t TSS_ACCESS = (AccessByte::PRESENT | AccessByte::RING0 | AccessByte::AVAILABLE_TSS);
constexpr uint8_t CODE_FLAG_ACCESS = (GdtFlags::GRANULARITY | GdtFlags::LONG_MODE);
constexpr uint8_t DATA_FLAG_ACCESS = (GdtFlags::GRANULARITY | GdtFlags::SIZE);

struct GdtEntry {
    uint16_t lowerLimit;
    uint16_t lowerBase;
    uint8_t midBase;
    uint8_t accessByte;
    uint8_t higherLimit_flags; // Lower 4 bits are the higher limit, upper 4 are flags
    uint8_t upperBase;
} __attribute__((packed));

struct GdtPointer {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct TssDescriptor {
    uint16_t lowerLimit;
    uint16_t lowerBase1;
    uint8_t lowerBase2;
    uint8_t accessByte;
    uint8_t higherLimit_flags; // Lower 4 bits are the higher limit, upper 4 are flags
    uint8_t upperBase1;
    uint32_t upperBase2;
    uint32_t _reserved1;
} __attribute__((packed));

struct Tss {
    uint32_t _reserved1;
    uint64_t rsp[3];
    uint64_t _reserved2;
    uint64_t ist[7]; // starts at IST1
    uint64_t _reserved3;
    uint16_t _reserved4;
    uint16_t ioMapBase;  
} __attribute__((packed));

struct Gdt {
    GdtEntry null;
    GdtEntry kernelCode;
    GdtEntry kernelData;
    GdtEntry userData;
    GdtEntry userCode;
    TssDescriptor tssDescriptor;
} __attribute__((packed));

constexpr uint16_t KERNEL_CODE_SELECTOR = offsetof(Gdt, kernelCode);
constexpr uint16_t KERNEL_DATA_SELECTOR = offsetof(Gdt, kernelData);
constexpr uint16_t USER_CODE_SELECTOR = offsetof(Gdt, userCode) | RplConstants::RPL3;
constexpr uint16_t USER_DATA_SELECTOR = offsetof(Gdt, userData) | RplConstants::RPL3;
constexpr uint16_t TSS_DESCRIPTOR_SELECTOR = offsetof(Gdt, tssDescriptor);

static_assert(sizeof(GdtEntry) == 8, "GDT Entry Size is Incorrect!");
static_assert(sizeof(GdtPointer) == 10, "LGDT Size is Incorrect!");
static_assert(sizeof(TssDescriptor) == 16, "TSS Descriptor Size is Incorrect!");
static_assert(sizeof(Tss) == 104, "TSS Size is Incorrect!");
static_assert(KERNEL_CODE_ACCESS == 0x9A, "Incorrect Kernel Code Byte");
static_assert(KERNEL_DATA_ACCESS == 0x92, "Incorrect Kernel Data Byte");
static_assert(USER_CODE_ACCESS == 0xFA, "Incorrect User Code Byte");
static_assert(USER_DATA_ACCESS == 0xF2, "Incorrect User Data Byte");
static_assert(TSS_ACCESS == 0x89, "Incorrect TSS Byte");
static_assert(CODE_FLAG_ACCESS == 0xA0, "Incorrect Code Flags");
static_assert(DATA_FLAG_ACCESS == 0xC0, "Incorrect Data Flags");
static_assert(sizeof(Gdt) == 56, "Incorrect Gdt Size");
static_assert(KERNEL_CODE_SELECTOR == 0x08, "Incorrect Kernel Code Offset");
static_assert(KERNEL_DATA_SELECTOR == 0x10, "Incorrect Kernel Data Offset");
static_assert(USER_DATA_SELECTOR == 0x1B, "Incorrect User Data Offset");
static_assert(USER_CODE_SELECTOR == 0x23, "Incorrect User Code Offset");
static_assert(TSS_DESCRIPTOR_SELECTOR == 0x28, "Incorrect Tss Descriptor Offset");

GdtEntry create_gdt_entry(uint8_t accessByte, uint32_t limit, uint8_t flags);
TssDescriptor create_tss_descriptor(uint64_t* tssAddress);
void initialize_tss(uint64_t kernelStackTop, uint64_t doubleFaultStackTop);
void gdt_init(uint64_t kernelStackTop, uint64_t doubleFaultStackTop);