#include "boot_info.h"
#include "tables/gdt.h"
#include "tables/idt.h"
#include "io/pic.h"
#include "io/keyboard.h"
#include "renderer/renderer.h"
#include "memory/memory_manager.h"

constexpr uint16_t KERNEL_STACK_SIZE = 16384;
constexpr uint16_t DOUBLE_FAULT_STACK_SIZE = 8192;

alignas(16) uint8_t stack[KERNEL_STACK_SIZE];
alignas(16) uint8_t doubleFaultStack[DOUBLE_FAULT_STACK_SIZE];

extern "C" [[noreturn]] void kernel_main(BootInfo* binfo) {
    gdt_init(reinterpret_cast<uint64_t>(&stack) + KERNEL_STACK_SIZE, reinterpret_cast<uint64_t>(&doubleFaultStack) + DOUBLE_FAULT_STACK_SIZE);
    idt_init();
    pic_init();

    asm volatile("sti");

    Renderer _renderer(binfo);
    MemoryManager _memManager(binfo);

    memManager = &_memManager;
	renderer = &_renderer;

    MemoryAccess memAccess = memManager->allocate_physical_page();
    memManager->free_physical_page(memAccess);

    renderer->clear(0xFFF);
    cursor_x = 100; cursor_y = 100; renderer->print_int(memAccess.address, 0);
    cursor_x = 100; cursor_y = 120; renderer->print_int(memAccess.key, 0);
    cursor_x = 100; cursor_y = 140; renderer->print_int(memAccess.pages, 0);
    renderer->blit();


    while (1) {
        if (keyboard.poll_event() != 0x00) {
            renderer->clear(0xFFFF);
            renderer->blit();
        }
    }

	for (;;) asm volatile("hlt");
}