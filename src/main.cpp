#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "cpu/cpu.h"
#include "memory/mem.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom.gb>\n";
        return 1;
    }

    std::ifstream rom_file(argv[1], std::ios::binary | std::ios::ate);
    if (!rom_file) {
        std::cerr << "Failed to open ROM: " << argv[1] << "\n";
        return 1;
    }

    std::streamsize size = rom_file.tellg();
    rom_file.seekg(0, std::ios::beg);

    RAM ram;

    if (size > static_cast<std::streamsize>(sizeof(ram.rom))) {
        std::cerr << "ROM too large for current mapper-less RAM (max "
                  << sizeof(ram.rom) << " bytes, got " << size << ")\n";
        return 1;
    }

    if (!rom_file.read(reinterpret_cast<char*>(ram.rom), size)) {
        std::cerr << "Failed to read ROM into memory\n";
        return 1;
    }

    CPU cpu(ram);
    cpu.init();
    cpu.reg.PC = 0x0100; // Standard Game Boy entry point after boot ROM

    // Simple run loop. No timers/PPU/joypad yet, so this just
    // steps the CPU. Add a cycle-accurate clock later if needed.
    const uint64_t MAX_STEPS = 100000000ULL;
    uint64_t steps = 0;

    while (steps < MAX_STEPS) {
        cpu.execute();
        ++steps;

        if (cpu.STOP) {
            std::cout << "CPU stopped after " << steps << " steps.\n";
            break;
        }
    }

    if (steps >= MAX_STEPS) {
        std::cout << "Hit max step count (" << MAX_STEPS << "), stopping.\n";
    }

    return 0;
}