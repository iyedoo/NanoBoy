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
    if (!file) { cerr << "Failed to open " << argv[1] << '\n'; return 1; }

    vector<uint8_t> data((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    if (!ram.load_rom(data)) { cerr << "Invalid ROM\n"; return 1; }
    cout << "ROM loaded: " << data.size() << " bytes\n";

    cpu.init();
    ram.write(0xFF0F, 0xE1);
    ram.write(0xFFFF, 0x00);

    FILE* logf = nullptr;
    if (argc > 2 && string(argv[2]) == "--log") logf = fopen("doctor.log", "w");

    uint64_t instructions = 0;

    while (instructions < 100000000) {
        if (logf) cpu.log(logf);

        uint16_t pc = cpu.reg.PC;
        cpu.execute();
        ++instructions;

        // Test ROMs finish in a `jr $` / `jp $` loop
        if (cpu.reg.PC == pc && !cpu.HALT) break;
    }

    if (logf) fclose(logf);
    cout << '\n';

    cerr << "\nStopped after " << instructions << " instructions\n";
    cerr << "PC = 0x" << hex << setw(4) << setfill('0') << cpu.reg.PC << '\n';

    return 0;
}