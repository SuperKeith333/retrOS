#pragma once

#include <cstdint>

typedef struct {
    uint32_t* frontBuffer;
    uint32_t* backBuffer;
    uint64_t screenWidth;
    uint64_t screenHeight;
    uint64_t pixelsPerScanline;
} BootInfo;