#include "cpu.h"

void REG::flags(bool z, bool n, bool h, bool c) {
    F = (z << 7) | (n << 6) | (h << 5) | (c << 4);
}

uint16_t REG::AF() { return (A << 8) | F; }
uint16_t REG::BC() { return (B << 8) | C; }
uint16_t REG::DE() { return (D << 8) | E; }
uint16_t REG::HL() { return (H << 8) | L; }

// void REG::sAF(uint16_t val) { A = (val >> 8), F = (val & 0b0000000011110000); }
// void REG::sBC(uint16_t val) { B = (val >> 8), C = (val & 0b0000000011111111); }
// void REG::sDE(uint16_t val) { D = (val >> 8), E = (val & 0b0000000011111111); }
// void REG::sHL(uint16_t val) { H = (val >> 8), L = (val & 0b0000000011111111); }

CPU::CPU(RAM& ram) : ram(ram) {}

void CPU::init() {
    reg.A = 0x00, reg.F = 0x00;
    reg.B = 0x00, reg.C = 0x00;
    reg.D = 0x00, reg.E = 0x00;
    reg.H = 0x00, reg.L = 0x00;

    reg.PC = 0x0000, reg.SP = 0x0000;
}

void CPU::ADD(uint8_t r) {
    bool C[9];

    uint8_t sum = 0;

    C[0] = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;
        
        bool S = A ^ R ^ C[i];
        C[i + 1] = (A & R) | (C[i] & (A ^ R));

        sum |= (S << i);
    }

    reg.A = sum;
    reg.flags(sum == 0, 0, C[4], C[8]);
}

void CPU::step() {
    uint8_t opcode = ram.mem[reg.PC];
    reg.PC += 0x0001;
    switch (opcode) {
        case 0x3E: // LD A, d8
            reg.A = ram.read(reg.PC);
            reg.PC += 0x0001;
            break;

        case 0x06: // lD B, d8
            reg.B = ram.read(reg.PC);
            reg.PC += 0x0001;
            break;

        case 0x80: // ADD A, B
            ADD(reg.B);
            break;

        case 0x81: // ADD A, C
            ADD(reg.C);
            break;

        case 0x82: // ADD A, D
            ADD(reg.D);
            break;

        case 0x83: // ADD A, E
            ADD(reg.E);
            break;

        case 0x84: // ADD A, H
            ADD(reg.H);
            break;

        case 0x85: // ADD A, L
            ADD(reg.L);
            break;

        case 0x86: // ADD A, (HL)
            ADD(ram.mem[(reg.HL())]);
            break;

        case 0x87: // ADD A, A
            ADD(reg.A);
            break;
    }
}