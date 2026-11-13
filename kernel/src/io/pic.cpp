#include "io/pic.h"
#include "io.h"

void pic_init() {
    outb(MASTER_COMMAND_PORT, 0x11); // ICW1 start init
    io_wait();
    outb(SLAVE_COMMAND_PORT, 0x11); // ICW1 start init
    io_wait();
    outb(MASTER_DATA_PORT, 0x20); // ICW2 vector offset 32
    io_wait();
    outb(SLAVE_DATA_PORT, 0x28); // ICW2 vector offset 40
    io_wait();
    outb(MASTER_DATA_PORT, 0x04); // ICW3 slave is on IRQ2 
    io_wait();
    outb(SLAVE_DATA_PORT, 0x02); // ICW3 slave’s cascade ID is 2
    io_wait();
    outb(MASTER_DATA_PORT, 0x01); // ICW4 8086 mode
    io_wait();
    outb(SLAVE_DATA_PORT, 0x01); // ICW4 8086 mode
    io_wait();
    outb(MASTER_DATA_PORT, 0xFD); // mask all IRQs masked except IRQ1
    io_wait();
    outb(SLAVE_DATA_PORT, 0xFF); // mask all IRQs masked
    io_wait();
}