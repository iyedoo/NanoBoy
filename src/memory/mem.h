#pragma once
#include <cstdint>
#include <vector>

class TIMER {
public:
    void connect(uint8_t* if_reg) { IF = if_reg; }

    void tick(uint64_t cycles);

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);

private:
    uint16_t DIV = 0;
    uint8_t TIMA = 0;
    uint8_t TMA = 0;
    uint8_t TAC = 0;

    uint8_t* IF = nullptr;

    uint8_t reload_delay = 0;
    uint8_t reload_window = 0;

    bool signal() const;
    void inc_tima();
    void update(uint16_t new_div, uint8_t new_tac);
};

class RAM {
public:
    TIMER timer;

    std::vector<uint8_t> rom;   // was uint8_t rom[0x8000]
    uint8_t vram[0x2000] = {0};
    uint8_t eram[0x8000] = {0}; // 4 banks of 8 KB
    uint8_t wram[0x2000] = {0};
    uint8_t oam[0xA0] = {};
    uint8_t io[0x80] = {};
    uint8_t hram[0x7F] = {};
    uint8_t ie = 0;

    // MBC1 state
    bool    mbc1 = false;
    uint32_t rom_banks = 2;
    bool    ram_enable = false;
    uint8_t bank1 = 1;   // 5-bit low ROM bank
    uint8_t bank2 = 0;   // 2-bit high ROM bank / RAM bank
    uint8_t mode = 0;

    RAM();
    RAM(const RAM&) = delete;
    RAM& operator=(const RAM&) = delete;

    bool load_rom(const std::vector<uint8_t>& data);
    void request_interrupt(uint8_t bit);

    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t val);

private:
    uint8_t if_reg = 0x01;
    void oam_dma(uint8_t page);
};