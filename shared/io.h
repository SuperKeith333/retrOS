#pragma once

#include <cstdint>

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void io_wait() {
    outb(0x80, 0);
}

static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi)); // Run the rdtsc instruction and store the lower 32 bits in lo and the upper 32 bits in hi
    return ((uint64_t)hi << 32) | lo; // returns to the total number of cpu cycles since the computer turned on
}