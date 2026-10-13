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
// --------------------------- 4000 |= 32kB Cartridge
// 16kB ROM bank #0 |
// --------------------------- 0000 --

#include "mem.h"
#include <algorithm>
#include <iostream>

bool TIMER::signal() const {
    if (!(TAC & 0x04)) return false;

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
    if (TIMA == 0) reload_delay = 4;
}

void TIMER::update(uint16_t new_div, uint8_t new_tac) {
    bool prev = signal();
    DIV = new_div;
    TAC = new_tac;
    if (prev && !signal()) inc_tima();
}

void TIMER::tick(uint64_t cycles) {
    for (uint64_t i = 0; i < cycles; ++i) {
        if (reload_delay > 0) {
            if (--reload_delay == 0) {
                TIMA = TMA;
                if (IF) *IF |= 0x04;
                reload_window = 4;
            }
        } else if (reload_window > 0) {
            --reload_window;
        }

        update(DIV + 1, TAC);
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
        case 0xFF04: update(0, TAC); break;
        case 0xFF05:
            if (reload_window > 0) break;
            TIMA = value;
            reload_delay = 0;
            break;
        case 0xFF06:
            TMA = value;
            if (reload_window > 0) TIMA = value;
            break;
        case 0xFF07: update(DIV, value & 0x07); break;
    }
}

RAM::RAM() {
    timer.connect(&if_reg);
}

bool RAM::load_rom(const std::vector<uint8_t>& data) {
    if (data.size() < 0x150) return false;

    uint8_t type = data[0x147];
    mbc1 = (type >= 0x01 && type <= 0x03);

    rom_banks = 2;
    while (rom_banks * 0x4000 < data.size()) rom_banks <<= 1;

    rom.assign(rom_banks * 0x4000, 0xFF);
    std::copy(data.begin(), data.end(), rom.begin());

    ram_enable = false; bank1 = 1; bank2 = 0; mode = 0;
    return true;
}

void RAM::request_interrupt(uint8_t bit) {
    if_reg |= (1 << bit) & 0x1F;
}

void RAM::oam_dma(uint8_t page) {
    uint16_t src = page << 8;
    for (int i = 0; i < 0xA0; ++i) oam[i] = read(src + i);
}

uint8_t RAM::read(uint16_t addr) {
    if (addr < 0x4000) {
        uint32_t bank = (mbc1 && mode) ? (bank2 << 5) : 0;
        return rom[(bank % rom_banks) * 0x4000 + addr];
    }
    if (addr < 0x8000) {
        uint32_t bank = mbc1 ? ((bank2 << 5) | bank1) : 1;
        return rom[(bank % rom_banks) * 0x4000 + (addr - 0x4000)];
    }
    if (addr < 0xA000) return vram[addr - 0x8000];
    if (addr < 0xC000) {
        if (!ram_enable) return 0xFF;
        uint32_t bank = (mbc1 && mode) ? bank2 : 0;
        return eram[bank * 0x2000 + (addr - 0xA000)];
    }
    if (addr < 0x8000) return rom[addr];
    if (addr < 0xA000) return vram[addr - 0x8000];
    if (addr < 0xC000) return eram[addr - 0xA000];
    if (addr < 0xE000) return wram[addr - 0xC000];
    if (addr < 0xFE00) return wram[addr - 0xE000];
    if (addr < 0xFEA0) return oam[addr - 0xFE00];
    if (addr < 0xFF00) return 0xFF;
    if (addr >= 0xFF04 && addr <= 0xFF07) return timer.read(addr);
    if (addr == 0xFF0F) return if_reg | 0xE0;
    if (addr < 0xFF80) return io[addr - 0xFF00];
    if (addr < 0xFFFF) return hram[addr - 0xFF80];

    return ie;
}

void RAM::write(uint16_t addr, uint8_t val) {
    if (addr < 0x8000) {
        if (!mbc1) return;
        if (addr < 0x2000)      ram_enable = (val & 0x0F) == 0x0A;
        else if (addr < 0x4000) { bank1 = val & 0x1F; if (bank1 == 0) bank1 = 1; }
        else if (addr < 0x6000) bank2 = val & 0x03;
        else                    mode = val & 0x01;
    }
    else if (addr < 0xA000) vram[addr - 0x8000] = val;
    else if (addr < 0xC000) {
        if (!ram_enable) return;
        uint32_t bank = (mbc1 && mode) ? bank2 : 0;
        eram[bank * 0x2000 + (addr - 0xA000)] = val;
    }
    else if (addr < 0xA000) vram[addr - 0x8000] = val;
    else if (addr < 0xC000) eram[addr - 0xA000] = val;
    else if (addr < 0xE000) wram[addr - 0xC000] = val;
    else if (addr < 0xFE00) wram[addr - 0xE000] = val;
    else if (addr < 0xFEA0) oam[addr - 0xFE00] = val;
    else if (addr < 0xFF00) return;
    else if (addr >= 0xFF04 && addr <= 0xFF07) timer.write(addr, val);
    else if (addr == 0xFF0F) if_reg = val & 0x1F;
    else if (addr == 0xFF02) {
        io[0x02] = val;
        if (val == 0x81) std::cout << (char)io[0x01] << std::flush;
    }
    else if (addr == 0xFF46) {
        io[0x46] = val;
        oam_dma(val);
    }
    else if (addr < 0xFF80) io[addr - 0xFF00] = val;
    else if (addr < 0xFFFF) hram[addr - 0xFF80] = val;
    else ie = val;
}