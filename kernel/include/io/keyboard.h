#pragma once

#include <cstdint>

constexpr uint8_t KEYBOARD_BUFFER_SIZE = 128;

class Keyboard {
public:
    Keyboard();

    void key_event_recieved(uint8_t event);
    uint8_t poll_event();

    volatile uint8_t buffer[KEYBOARD_BUFFER_SIZE]{};
    uint8_t bufferSize = 0;
};

inline Keyboard keyboard;