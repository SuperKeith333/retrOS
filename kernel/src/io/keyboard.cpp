#include "io/keyboard.h"

Keyboard::Keyboard() {

}

void Keyboard::key_event_recieved(uint8_t event) {
    buffer[bufferSize] = event;
    bufferSize++;
}

uint8_t Keyboard::poll_event() {
    if (bufferSize <= 0)
        return 0x00;

    uint8_t event = buffer[0];
    for (uint8_t i = 1; i < bufferSize - 1; i++) {
        buffer[i - 1] = buffer[i];
    }

    for (uint8_t i = bufferSize; i < KEYBOARD_BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }

    return event;
}