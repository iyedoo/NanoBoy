#pragma once
#include <cstdint>
// #include "../cpu/cpu.h"

class TIMER {
public:
    void tick(uint64_t cycles);

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);

private:
    uint16_t DIV = 0;
    uint8_t TIMA = 0;
    uint8_t TMA = 0;
    uint8_t TAC = 0;

    bool signal() const;
    void inc_tima();
};

class RAM {
public:
    TIMER timer;

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