#include "renderer/renderer.h"
#include "renderer/font.h"

Renderer::Renderer(BootInfo* bInfo) {
    frontBuffer = bInfo->frontBuffer;
    backBuffer = bInfo->backBuffer;
    totalPixels = bInfo->pixelsPerScanline * bInfo->screenHeight;
    pixelsPerScanline = bInfo->pixelsPerScanline;
}

void Renderer::blit() {
    for (unsigned long long i = 0; i < totalPixels; i++) {
        frontBuffer[i] = backBuffer[i];
    }
}

void Renderer::clear(uint32_t color) {
    for (unsigned long long i = 0; i < totalPixels; i++) {
        backBuffer[i] = color;
    }
}
void Renderer::putchar(char c, uint32_t color) {
    unsigned char character_index = (unsigned char)c;
    unsigned char* glyph = &font_bin[character_index * 16];

    for (int y = 0; y < 16; y++) {
        unsigned char font_row = glyph[y];
        for (int x = 0; x < 8; x++) {
            if ((font_row << x) & 0x80) {
                unsigned long long offset = (cursor_x + x) + ((cursor_y + y) * pixelsPerScanline);
                backBuffer[offset] = color;
            }
        }
    }
    cursor_x += 8;
}

void Renderer::print(const char* str, uint32_t color) {
    while (*str) {
        if (*str == '\n') {
            cursor_x = 50;
            cursor_y += 20;
        } else {
            putchar(*str, color);
        }
        str++;
    }
}

void Renderer::print_int(unsigned long long num, uint32_t color) {
    char buffer[21];
    int i = 19;
    buffer[20] = '\0';
    
    if (num == 0) {
        buffer[i--] = '0';
    } else {
        while (num > 0 && i >= 0) {
            buffer[i--] = (num % 10) + '0';
            num /= 10;
        }
    }
    print(&buffer[i + 1], color);
}