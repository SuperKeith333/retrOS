#include "idt/idt.h"
#include "io.h"
#include "io/keyboard.h"
#include "renderer/renderer.h"

alignas(16) static IdtGateDescriptor idtGateDescriptors[256];
static IdtPointer idtPointer;
volatile bool int3Ran = false;

static_assert(sizeof(idtGateDescriptors) == 4096, "Incorrect IDT Gate Descriptors Size");

IdtGateDescriptor create_idt_gate_descriptor(uint64_t handlerAddress, uint8_t istIndex, uint8_t attributeByte) {
    IdtGateDescriptor idtGateDescriptor{};

    idtGateDescriptor.lowerOffset = static_cast<uint16_t>(handlerAddress);
    idtGateDescriptor.upperOffset1 = static_cast<uint16_t>(handlerAddress >> 16);
    idtGateDescriptor.upperOffset2 = static_cast<uint32_t>(handlerAddress >> 32);
    idtGateDescriptor.segmentSelector = KERNEL_CODE_SELECTOR;
    idtGateDescriptor.ist_reserved1 = static_cast<uint8_t>(istIndex & 0x7);
    idtGateDescriptor.gateType_zeroBit_dpl_present = attributeByte;

    return idtGateDescriptor;
}

void set_idt_gate(uint8_t index, uint64_t handlerAddress, uint8_t istIndex, uint8_t attributeByte) {
    idtGateDescriptors[index] = create_idt_gate_descriptor(handlerAddress, istIndex, attributeByte);
}

__attribute__((interrupt)) void int0Handler(ExceptionValues* values) {
    // Divide Error - report Rip from values then halt

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int3Handler(ExceptionValues* values) {
    // BreakPoint - report and return

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("Breakpoint hit!", 0xFFFFFF);
    cursor_x = 550; cursor_y = 120; renderer->print("Press Any Key To Continue!", 0xFFFFFF);
    renderer->blit();

    while ((inb(0x64) & 0x01) == 0) {}
}

__attribute__((interrupt)) void int6Handler(ExceptionValues* values) {
    // Invalid Optcode - report Rip and halt

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int8Handler(ExceptionValues* values, uint64_t errorCode) {
    // Double Fault - report, halt, never return


    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int13Handler(ExceptionValues* values, uint64_t errorCode) {
    // General Protection - report Rip and errorCode, halt

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int14Handler(ExceptionValues* values, uint64_t errorCode) {
    // Page Fault - report Rip, error code, and cr2, halt
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int33Handler(ExceptionValues* values) {
    // Keyboard Interrupt
    if (inb(0x64) & 0x01)
        keyboard.key_event_recieved(inb(0x60));
}

void idt_init() {
    set_idt_gate(0, reinterpret_cast<uint64_t>(int0Handler), 0, KERNEL_INTERRUPT_GATE);
    set_idt_gate(3, reinterpret_cast<uint64_t>(int3Handler), 0, KERNEL_INTERRUPT_GATE);
    set_idt_gate(6, reinterpret_cast<uint64_t>(int6Handler), 0, KERNEL_INTERRUPT_GATE);
    set_idt_gate(8, reinterpret_cast<uint64_t>(int8Handler), 1, KERNEL_INTERRUPT_GATE);
    set_idt_gate(13, reinterpret_cast<uint64_t>(int13Handler), 0, KERNEL_INTERRUPT_GATE);
    set_idt_gate(14, reinterpret_cast<uint64_t>(int14Handler), 0, KERNEL_INTERRUPT_GATE);
    set_idt_gate(33, reinterpret_cast<uint64_t>(int33Handler), 0, KERNEL_INTERRUPT_GATE);

    idtPointer.size = sizeof(idtGateDescriptors) - 1;
    idtPointer.offset = reinterpret_cast<uint64_t>(idtGateDescriptors);

    asm volatile("lidt %0" : : "m"(idtPointer) : "memory");
}