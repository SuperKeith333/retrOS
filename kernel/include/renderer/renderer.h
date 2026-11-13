#pragma once

#include "boot_info.h"

class Renderer {
public:
    Renderer(BootInfo* bInfo);

    void blit();
    void clear(uint32_t color);
    void putchar(char c, uint32_t color);
    void print(const char* str, uint32_t color);
    void print_int(unsigned long long num, uint32_t color);

private:
    uint32_t* frontBuffer;
    uint32_t* backBuffer;
    unsigned long long totalPixels;
    uint64_t pixelsPerScanline;
};

inline Renderer* renderer;
inline uint32_t cursor_x = 50;
inline uint32_t cursor_y = 50;