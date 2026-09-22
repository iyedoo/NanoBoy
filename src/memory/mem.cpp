// Interrupt Enable Register
// --------------------------- FFFF
// Internal RAM
// --------------------------- FF80
// Empty but unusable for I/O
// --------------------------- FF4C
// I/O ports
// --------------------------- FF00
// Empty but unusable for I/O
// --------------------------- FEA0
// Sprite Attrib Memory (OAM)
// --------------------------- FE00
// Echo of 8kB Internal RAM
// --------------------------- E000
// 8kB Internal RAM
// --------------------------- C000
// 8kB switchable RAM bank
// --------------------------- A000
// 8kB Video RAM
// --------------------------- 8000 --
// 16kB switchable ROM bank |
// --------------------------- 4000 |= 32kB Cartrigbe
// 16kB ROM bank #0 |
// --------------------------- 0000 --

#include "mem.h"
#include <iostream>

bool TIMER::signal() const {
    if (!(TAC & 0x04)) return 0;

    int bit;

    switch (TAC & 0x03) {
        case 0: bit = 9; break;
        case 1: bit = 3; break;
        case 2: bit = 5; break;
        default: bit = 7; break;
    }

    return (DIV >> bit) & 1;
}

void TIMER::inc_tima() {
    ++TIMA;
    if (TIMA == 0) TIMA = TMA;
}

void TIMER::tick(uint64_t cycles) {
    for (int i = 0; i < cycles; ++i) {
        bool prev = signal();
        
        ++DIV;

        bool curr = signal();
        if (prev && !curr) inc_tima();
    }
}

uint8_t TIMER::read(uint16_t addr) const {
    switch (addr) {
        case 0xFF04: return DIV >> 8;
        case 0xFF05: return TIMA;
        case 0xFF06: return TMA;
        case 0xFF07: return TAC | 0xF8;
        default: return 0xFF;
    }
}

void TIMER::write(uint16_t addr, uint8_t value) {
    switch (addr) {
        case 0xFF04: DIV = 0; break;
        case 0xFF05: TIMA = value; break;
        case 0xFF06: TMA = value; break;
        case 0xFF07: TAC = value & 0x07; break;
    }
}

RAM::RAM() {}

uint8_t RAM::read(uint16_t addr) {
    if (addr < 0x8000) return rom[addr];
    if (addr < 0xA000) return vram[addr - 0x8000];
    if (addr < 0xC000) return eram[addr - 0xA000];
    if (addr < 0xE000) return wram[addr - 0xC000];
    if (addr < 0xFE00) return wram[addr - 0xE000];
    if (addr < 0xFEA0) return oam[addr - 0xFE00];
    if (addr < 0xFF00) return 0xFF;
    if (addr >= 0xFF04 && addr <= 0xFF07) return timer.read(addr);
    if (addr < 0xFF80) return io[addr - 0xFF00];
    if (addr < 0xFFFF) return hram[addr - 0xFF80];

    return ie;
}

void RAM::write(uint16_t addr, uint8_t val) {
    if (addr < 0x8000) rom[addr] = val;
    else if (addr < 0xA000) vram[addr - 0x8000] = val;
    else if (addr < 0xC000) eram[addr - 0xA000] = val;
    else if (addr < 0xE000) wram[addr - 0xC000] = val;
    else if (addr < 0xFE00) wram[addr - 0xE000] = val;
    else if (addr < 0xFEA0) oam[addr - 0xFE00] = val;
    else if (addr < 0xFF00) return;
    else if (addr >= 0xFF04 && addr <= 0xFF07) timer.write(addr, val);
    else if (addr == 0xFF02) {
        io[0x02] = val;
        if (val == 0x81) std::cout << (char)io[0x01] << std::flush;
    }
    else if (addr < 0xFF80) io[addr - 0xFF00] = val;
    else if (addr < 0xFFFF) hram[addr - 0xFF80] = val;
    else ie = val;
}