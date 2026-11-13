#pragma once

#include <cstdint>
#include "gdt.h"

namespace IdtGates {
    constexpr uint8_t INTERRUPT_GATE = 0xE;
    constexpr uint8_t TRAP_GATE = 0xF;
}

constexpr uint8_t KERNEL_INTERRUPT_GATE = AccessByte::PRESENT | AccessByte::RING0 | IdtGates::INTERRUPT_GATE;
constexpr uint8_t KERNEL_TRAP_GATE = AccessByte::PRESENT | AccessByte::RING0 | IdtGates::TRAP_GATE;
constexpr uint8_t USER_INTERRUPT_GATE = AccessByte::PRESENT | AccessByte::RING3 | IdtGates::INTERRUPT_GATE;

struct ExceptionValues {
    uint64_t Rip;
    uint64_t Cs;
    uint64_t RFlags;
    uint64_t Rsp;
    uint64_t SS;
} __attribute__((packed));

struct IdtPointer {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed));

struct IdtGateDescriptor {
    uint16_t lowerOffset;
    uint16_t segmentSelector;
    uint8_t ist_reserved1;
    uint8_t gateType_zeroBit_dpl_present;
    uint16_t upperOffset1;
    uint32_t upperOffset2;
    uint32_t _reserved2;
} __attribute__((packed));

static_assert(sizeof(IdtGateDescriptor) == 16, "Incorrect Idt Gate Descriptor Size");
static_assert(sizeof(IdtPointer) == 10, "Incorrect IDT Pounter Size");
static_assert(sizeof(ExceptionValues) == 40, "Incorrect Exception Values Size");
static_assert(KERNEL_INTERRUPT_GATE == 0x8E, "Incorrect Kernel Interrupt Gate");
static_assert(KERNEL_TRAP_GATE == 0x8F, "Incorrect Kernel Trap Gate");
static_assert(USER_INTERRUPT_GATE == 0xEE, "Incorrect User Interrupt Gate");

IdtGateDescriptor create_idt_gate_descriptor(uint64_t handlerAddress, uint8_t istIndex, uint8_t attributeByte);
void set_idt_gate(uint8_t index, uint64_t handlerAddress, uint8_t istIndex, uint8_t attributeByte);

__attribute__((interrupt)) void int0Handler(ExceptionValues* values); // Divide Error
__attribute__((interrupt)) void int3Handler(ExceptionValues* values); // BreakPoint
__attribute__((interrupt)) void int6Handler(ExceptionValues* values); // Invalid Optcode
__attribute__((interrupt)) void int8Handler(ExceptionValues* values, uint64_t errorCode); // Double Fault
__attribute__((interrupt)) void int13Handler(ExceptionValues* values, uint64_t errorCode); // General Protection
__attribute__((interrupt)) void int14Handler(ExceptionValues* values, uint64_t errorCode); // Page Fault
__attribute__((interrupt)) void int33Handler(ExceptionValues* values); // Keyboard Interrupt

void idt_init();