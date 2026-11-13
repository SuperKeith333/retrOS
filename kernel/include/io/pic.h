#pragma once

#include <cstdint>

constexpr uint8_t MASTER_COMMAND_PORT = 0x20;
constexpr uint8_t SLAVE_COMMAND_PORT = 0xA0;
constexpr uint8_t MASTER_DATA_PORT = 0x21;
constexpr uint8_t SLAVE_DATA_PORT = 0xA1;

void pic_init();