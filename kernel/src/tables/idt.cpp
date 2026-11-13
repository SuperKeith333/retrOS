#include "tables/idt.h"
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

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("Divide Error!", 0xFFFFFF);
    cursor_x = 480; cursor_y = 120; renderer->print("Address of faulting instruction: ", 0xFFFFFF);
    cursor_x = 750; cursor_y = 120; renderer->print_int(values->Rip, 0xFFFFFF);
    renderer->blit();

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

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("Invaid Optcode!", 0xFFFFFF);
    cursor_x = 480; cursor_y = 120; renderer->print("Address of faulting instruction: ", 0xFFFFFF);
    cursor_x = 750; cursor_y = 120; renderer->print_int(values->Rip, 0xFFFFFF);
    renderer->blit();

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int8Handler(ExceptionValues* values, uint64_t errorCode) {
    // Double Fault - report, halt, never return

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("Double Fault!", 0xFFFFFF);
    renderer->blit();

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int13Handler(ExceptionValues* values, uint64_t errorCode) {
    // General Protection - report Rip and errorCode, halt

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("General Protection!", 0xFFFFFF);
    cursor_x = 480; cursor_y = 120; renderer->print("Address of faulting instruction: ", 0xFFFFFF);
    cursor_x = 750; cursor_y = 120; renderer->print_int(values->Rip, 0xFFFFFF);
    cursor_x = 600; cursor_y = 140; renderer->print("Error Code:", 0xFFFFFF);
    cursor_x = 700; cursor_y = 140; renderer->print_int(errorCode, 0xFFFFFF);
    renderer->blit();

    for (;;) asm volatile("hlt");
}

__attribute__((interrupt)) void int14Handler(ExceptionValues* values, uint64_t errorCode) {
    // Page Fault - report Rip, error code, and cr2, halt
    uint64_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));

    renderer->clear(0);
    cursor_x = 590; cursor_y = 100; renderer->print("Page Fault!", 0xFFFFFF);
    cursor_x = 480; cursor_y = 120; renderer->print("Address of faulting instruction: ", 0xFFFFFF);
    cursor_x = 750; cursor_y = 120; renderer->print_int(values->Rip, 0xFFFFFF);
    cursor_x = 580; cursor_y = 140; renderer->print("Error Code:", 0xFFFFFF);
    cursor_x = 680; cursor_y = 140; renderer->print_int(errorCode, 0xFFFFFF);
    cursor_x = 580; cursor_y = 160; renderer->print("CR2:", 0xFFFFFF);
    cursor_x = 620; cursor_y = 160; renderer->print_int(cr2, 0xFFFFFF);
    renderer->blit();

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