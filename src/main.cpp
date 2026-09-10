#include "cpu/cpu.h"
#include "memory/mem.h"

#include <iostream>
#include <iomanip>

int main() {
    RAM ram;
    CPU cpu(ram);

    cpu.init();

    ram.write(0x0000, 0x3E); // LD A, d8
    ram.write(0x0001, 0x42); // A = 0x42

    ram.write(0x0002, 0x06); // LD B, d8
    ram.write(0x0003, 0x18); // B = 0x18

    ram.write(0x0004, 0x80); // ADD A, B

    cpu.execute();
    cpu.execute();
    cpu.execute();

    std::cout << "A = 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.reg.A) << '\n';
    std::cout << "F = 0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.reg.F) << '\n';
    std::cout << "PC = 0x" << std::hex << std::setw(4) << std::setfill('0') << cpu.reg.PC << '\n';
    return 0;
}