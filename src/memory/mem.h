#pragma once
#include <cstdint>

class RAM {
public:
    uint8_t mem[1 << 16];

    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t val);
};