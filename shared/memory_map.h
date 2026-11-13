#pragma once

struct MemoryMapEntry {
    unsigned int type;
    unsigned int pad;
    unsigned long long physicalStart; // Address on physical stick of ram
    unsigned long long virtualStart; // Address according to OS (unused)
    unsigned long long numberOfPages;
    unsigned long long attribute;
};