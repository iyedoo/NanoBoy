#include "cpu/cpu.h"
#include "memory/mem.h"

#include <iostream>

int main() {
    RAM ram;
    CPU cpu(ram);

    cpu.init();

    ram.mem[0x0000] = 0x3E;
    ram.mem[0x0001] = 0x42;
    ram.mem[0x0002] = 0x06;
    ram.mem[0x0003] = 0x05;
    ram.mem[0x0004] = 0x80;
    ram.mem[0x0005] = 0x00;

    cpu.step();

    return 0;
}