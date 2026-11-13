#include "memory/memory_manager.h"
#include "renderer/renderer.h"
#include "io.h"

MemoryManager::MemoryManager(BootInfo* binfo) {
    unsigned long long numberOfDescriptors = binfo->memoryMapSize / binfo->descriptorSize;
    for (unsigned long long  i = 0; i < numberOfDescriptors; i++) {
        unsigned char* memoryMapInBytes = ((unsigned char*)binfo->memoryMap) + (i * binfo->descriptorSize);
        MemoryMapEntry* adjustedMemoryMap = (MemoryMapEntry*)memoryMapInBytes;

        if (adjustedMemoryMap->type == 7 && adjustedMemoryMap->physicalStart != 0) {
            if (usableRegions < MAX_MEMORY_REGIONS && bitmapUsedBytes + (adjustedMemoryMap->numberOfPages + 7) / 8 <= 4096) {
                memoryRegions[usableRegions].original = *adjustedMemoryMap;
                memoryRegions[usableRegions].nextFreeAddress = adjustedMemoryMap->physicalStart;
                memoryRegions[usableRegions].remainingPages = adjustedMemoryMap->numberOfPages;
                memoryRegions[usableRegions].regionStart = adjustedMemoryMap->physicalStart;
                memoryRegions[usableRegions].regionPages = adjustedMemoryMap->numberOfPages;
                memoryRegions[usableRegions].bitmapBytes = (adjustedMemoryMap->numberOfPages + 7) / 8;
                memoryRegions[usableRegions].bitmapOffset = bitmapUsedBytes;
                memoryRegions[usableRegions].key = 0;
                bitmapUsedBytes += memoryRegions[usableRegions].bitmapBytes;
                usableRegions++;
                freePages += adjustedMemoryMap->numberOfPages;
            }
        }
    }
}

uint64_t MemoryManager::GenerateRandom64() {
    static uint64_t state = rdtsc(); 
        
    // Mix in fresh TSC entropy on every call
    state ^= rdtsc(); 
        
    uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

MemoryAccess MemoryManager::allocate_physical_page() {
    MemoryAccess memAccess = {};
    unsigned long long pagePhysicalStart = 0;
    for (unsigned long long i = 0; i < usableRegions; i++) {
        if (memoryRegions[i].remainingPages > 0) {
            for (unsigned long long pageIndex = 0; pageIndex < memoryRegions[i].regionPages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                if (!(bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    bitmapStorage[globalBitmapIndex] = (bitmapStorage[globalBitmapIndex] | (1 << bitIndex));
                    memoryRegions[i].remainingPages--;
                    freePages--;

                    pagePhysicalStart = memoryRegions[i].regionStart + (pageIndex * 4096);
                    memAccess.address = pagePhysicalStart;
                    memAccess.key = GenerateRandom64();
                    memAccess.pages = 1;

                    memoryRegions[i].key = memAccess.key;

                    return memAccess;
                }
            }
        }
    }

    renderer->clear(0);
    cursor_x = 560; cursor_y = 100; renderer->print("Failed to Allocate Page!", 0xFFFFFF);
    cursor_x = 580; cursor_y = 120; renderer->print("No Avaliable Pages!", 0xFFFFFF);
    renderer->blit();

    for (;;) asm volatile("hlt");

    return memAccess;
}

MemoryAccess MemoryManager::allocate_continuous_pages(uint32_t pages) {
    MemoryAccess memAccess = {};

    if (pages == 0) {
        renderer->clear(0);
        cursor_x = 500; cursor_y = 100; renderer->print("Failed to Allocate Continuous Pages!", 0xFFFFFF);
        cursor_x = 600; cursor_y = 120; renderer->print("Pages = 0", 0xFFFFFF);
        renderer->blit();

        for (;;) asm volatile("hlt");
        return memAccess;
    }

    unsigned long long pagePhysicalStart = 0;
    for (unsigned long long i = 0; i < usableRegions; i++) {
        if (memoryRegions[i].remainingPages >= pages) {
            uint32_t freePages = 0;
            uint32_t startingPage = 0;

            for (uint32_t pageIndex = 0; pageIndex < memoryRegions[i].regionPages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                if (!(bitmapStorage[globalBitmapIndex] & (1 << bitIndex))) {
                    freePages++;
                    if (freePages >= pages) {
                        startingPage = pageIndex - pages + 1;
                        break;
                    }
                } else {
                    freePages = 0;
                }
            }

            if (freePages != pages)
                continue;
            
            uint64_t key = GenerateRandom64();

            for (unsigned long long pageIndex = startingPage; pageIndex < startingPage + pages; pageIndex++) {
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                    
                bitmapStorage[globalBitmapIndex] = (bitmapStorage[globalBitmapIndex] | (1 << bitIndex));  
                memoryRegions[i].remainingPages--;    
                memoryRegions[i].key = key;
                freePages--;

                if (pageIndex == startingPage)
                    pagePhysicalStart = memoryRegions[i].regionStart + (pageIndex * 4096);
                
                
            }

            memAccess.address = pagePhysicalStart;
            memAccess.key = key;
            memAccess.pages = pages;

            return memAccess;
        }
    }

    renderer->clear(0);
    cursor_x = 560; cursor_y = 100; renderer->print("Failed to Allocate Page!", 0xFFFFFF);
    cursor_x = 580; cursor_y = 120; renderer->print("No Avaliable Pages!", 0xFFFFFF);
    renderer->blit();

    for (;;) asm volatile("hlt");

    return memAccess;
}

void MemoryManager::free_continuous_pages(MemoryAccess memAccesss) {
    if (memAccesss.key == 0 || memAccesss.address == 0 || memAccesss.pages == 0) {
        renderer->clear(0);
        cursor_x = 500; cursor_y = 100; renderer->print("Failed to Free Continuous Pages!", 0xFFFFFF);
        if (memAccesss.key == 0) {
            cursor_x = 560; cursor_y = 120; renderer->print("memAccess.key = 0", 0xFFFFFF);
        }
        if (memAccesss.address == 0) {
            cursor_x = 550; cursor_y = 140; renderer->print("memAccess.address = 0", 0xFFFFFF);
        }
        if (memAccesss.pages == 0) {
            cursor_x = 560; cursor_y = 160; renderer->print("memAccess.pages = 0", 0xFFFFFF);
        }
        renderer->blit();
        for (;;) asm volatile("hlt");
        return;
    }

    for (unsigned long long i = 0; i < usableRegions; i++) {
        if (memAccesss.address >= memoryRegions[i].regionStart && memAccesss.address < memoryRegions[i].regionStart + (memoryRegions[i].regionPages * 4096)) {
            bool allPagesAllocated = 1;
            uint64_t offset = memAccesss.address - memoryRegions[i].regionStart;

            if (offset / 4096 + memAccesss.pages > memoryRegions[i].regionPages)
                continue;

            uint32_t endingPage = offset / 4096 + memAccesss.pages;

            for (uint32_t pageIndex = offset / 4096; pageIndex < endingPage; pageIndex++) {

                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                if (!(bitmapStorage[globalBitmapIndex] & (1 << bitIndex)) || memoryRegions[i].key != memAccesss.key) {
                    allPagesAllocated = 0;
                }
            }
                
            if (allPagesAllocated) {
                for (uint32_t pageIndex = offset / 4096; pageIndex < endingPage; pageIndex++) {
                    unsigned long long byteIndex = pageIndex / 8;
                    unsigned long long bitIndex = pageIndex % 8;
                    unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                    bitmapStorage[globalBitmapIndex] = (bitmapStorage[globalBitmapIndex] & ~(1 << bitIndex));
                    memoryRegions[i].remainingPages++;
                    freePages++;
\
                    memoryRegions[i].key = 0;
                }

                return;
            }

        }
    }

    renderer->clear(0);
    cursor_x = 500; cursor_y = 100; renderer->print("Failed to Free Continuous Pages!", 0xFFFFFF);
    cursor_x = 540; cursor_y = 120; renderer->print("Found no pages to free!", 0xFFFFFF);
    renderer->blit();
    for (;;) asm volatile("hlt");
}

void MemoryManager::free_physical_page(MemoryAccess memAccesss) {
    if (memAccesss.key == 0 || memAccesss.address == 0 || memAccesss.pages == 0 || memAccesss.pages > 1) {
        renderer->clear(0);
        cursor_x = 500; cursor_y = 100; renderer->print("Failed to Free Page!", 0xFFFFFF);
        if (memAccesss.key == 0) {
            cursor_x = 500; cursor_y = 120; renderer->print("memAccess.key = 0", 0xFFFFFF);
        }
        if (memAccesss.address == 0) {
            cursor_x = 490; cursor_y = 140; renderer->print("memAccess.address = 0", 0xFFFFFF);
        }
        if (memAccesss.pages == 0) {
            cursor_x = 500; cursor_y = 160; renderer->print("memAccess.pages = 0", 0xFFFFFF);
        }
        if (memAccesss.pages > 1) {
            cursor_x = 500; cursor_y = 160; renderer->print("memAccess.pages > 1", 0xFFFFFF);
        }
        renderer->blit();
        for (;;) asm volatile("hlt");
        return;
    }
    if (memAccesss.address % 4096 == 0) {
        for (unsigned long long i = 0; i < usableRegions; i++) {
            unsigned long long physicalEnd = memoryRegions[i].regionStart + (memoryRegions[i].regionPages * 4096);
            if (memAccesss.address >= memoryRegions[i].regionStart && memAccesss.address < physicalEnd) {
                unsigned long long pageIndex = (memAccesss.address - memoryRegions[i].regionStart) / 4096;
                unsigned long long byteIndex = pageIndex / 8;
                unsigned long long bitIndex = pageIndex % 8;
                unsigned long long globalBitmapIndex = memoryRegions[i].bitmapOffset + byteIndex;
                
                if (bitmapStorage[globalBitmapIndex] & (1 << bitIndex) && memoryRegions[i].key == memAccesss.key) {
                    bitmapStorage[globalBitmapIndex] = (bitmapStorage[globalBitmapIndex] & ~(1 << bitIndex));

                    memoryRegions[i].remainingPages++;
                    memoryRegions[i].key = 0;
                    freePages++;
                    return;
                }
            }
        
        }
    }

    renderer->clear(0);
    cursor_x = 500; cursor_y = 100; renderer->print("Failed to Free Page!", 0xFFFFFF);
    cursor_x = 490; cursor_y = 120; renderer->print("Found no pages to free!", 0xFFFFFF);
    renderer->blit();
    for (;;) asm volatile("hlt");
}