#pragma once

#include <cstdint>

#include "boot_info.h"
#include "memory_map.h"

#define MAX_MEMORY_REGIONS 64

struct MemoryRegion {
    MemoryMapEntry original;
    uint64_t nextFreeAddress;
    uint64_t remainingPages;
    uint64_t regionStart;
    uint64_t regionPages;
    uint64_t bitmapOffset;
    uint64_t bitmapBytes;
    uint64_t key;
};

struct MemoryAccess {
    uint64_t address = 0;
    uint64_t key = 0;
    uint32_t pages = 0;
};

class MemoryManager {
public:
    MemoryManager(BootInfo* binfo);
    void free_physical_page(MemoryAccess memAccesss);
    void free_continuous_pages(MemoryAccess memAccesss);
    MemoryAccess allocate_physical_page();
    MemoryAccess allocate_continuous_pages(uint32_t pages);
private:
    MemoryRegion memoryRegions[MAX_MEMORY_REGIONS] = {};
    uint8_t usableRegions = 0;
    uint64_t freePages = 0;
    uint8_t bitmapStorage[8192] = {}; // Used to store which memory pages are allocated or free
    uint64_t bitmapUsedBytes = 0;

    uint64_t GenerateRandom64();
};

inline MemoryManager* memManager;