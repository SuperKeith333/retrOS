#pragma once

#include <cstdint>

#include "memory_map.h"

typedef struct {
    uint32_t* frontBuffer;
    uint32_t* backBuffer;
    uint64_t screenWidth;
    uint64_t screenHeight;
    uint64_t pixelsPerScanline;
    MemoryMapEntry* memoryMap;
    uint64_t memoryMapSize;
    uint64_t descriptorSize;
} BootInfo;