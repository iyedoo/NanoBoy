#include <bits/stdc++.h>

#include "cpu/cpu.h"
#include "memory/mem.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <rom>\n";
        return 1;
    }

    RAM ram;
    CPU cpu(ram);

    ifstream file(argv[1], ios::binary);

    if (!file) {
        cerr << "Failed to open " << argv[1] << '\n';
        return 1;
    }

    file.read(
        reinterpret_cast<char*>(ram.rom),
        sizeof(ram.rom)
    );

    cout << "ROM loaded: " << file.gcount() << " bytes\n";

    cpu.init();

    cpu.reg.A = 0x01;
    cpu.reg.F = 0xB0;
    cpu.reg.B = 0x00;
    cpu.reg.C = 0x13;
    cpu.reg.D = 0x00;
    cpu.reg.E = 0xD8;
    cpu.reg.H = 0x01;
    cpu.reg.L = 0x4D;

    cpu.reg.SP = 0xFFFE;
    cpu.reg.PC = 0x0100;

    ram.write(0xFF0F, 0xE1);
    ram.write(0xFFFF, 0x00);

    uint64_t instructions = 0;

    while (instructions < 100000000) {
        cpu.execute();
        ++instructions;
    }

    cerr << "\nStopped after " << instructions << " instructions\n";
    cerr << "PC = 0x"
         << hex << setw(4) << setfill('0')
         << cpu.reg.PC << '\n';

    return 0;
}