#pragma once
#include <cstdint>

class RAM {
public:
    uint8_t rom[0x8000] = {0};
    uint8_t vram[0x2000] = {0};
    uint8_t eram[0x2000] = {0};
    uint8_t wram[0x2000] = {0};
    uint8_t oam[0xA0] = {};
    uint8_t io[0x80] = {};
    uint8_t hram[0x7F] = {};
    uint8_t ie = 0;

    RAM();
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t val);
};