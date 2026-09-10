#include "cpu.h"

void REG::flags(bool z, bool n, bool h, bool c) {
    F = (z << 7) | (n << 6) | (h << 5) | (c << 4);
}

uint16_t REG::AF() { return (A << 8) | F; }
uint16_t REG::BC() { return (B << 8) | C; }
uint16_t REG::DE() { return (D << 8) | E; }
uint16_t REG::HL() { return (H << 8) | L; }

void REG::sAF(uint16_t val) { A = (val >> 8), F = (val & 0b0000000011110000); }
void REG::sBC(uint16_t val) { B = (val >> 8), C = (val & 0b0000000011111111); }
void REG::sDE(uint16_t val) { D = (val >> 8), E = (val & 0b0000000011111111); }
void REG::sHL(uint16_t val) { H = (val >> 8), L = (val & 0b0000000011111111); }

CPU::CPU(RAM& ram) : ram(ram) {}

void CPU::init() {
    reg.A = 0b00000000, reg.F = 0b00000000;
    reg.B = 0b00000000, reg.C = 0b00000000;
    reg.D = 0b00000000, reg.E = 0b00000000;
    reg.H = 0b00000000, reg.L = 0b00000000;

    reg.PC = 0b0000000000000000, reg.SP = 0b0000000000000000;
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

void CPU::ADC(uint8_t r) {
    bool C[9];

    uint8_t sum = 0;

    C[0] = (reg.F & 0b00010000);
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

void CPU::SUB(uint8_t r) {
    bool B[9];

    uint8_t diff = 0;

    B[0] = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;

        bool D = A ^ R ^ B[i];
        B[i + 1] = (!A & R) | (B[i] & !(A ^ R));

        diff |= (D << i);
    }

    reg.A = diff;
    reg.flags(diff == 0, 1, B[4], B[8]);
}

void CPU::SBC(uint8_t r) {
    bool B[9];

    uint8_t diff = 0;

    B[0] = reg.F & 0b00010000;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;

        bool D = A ^ R ^ B[i];
        B[i + 1] = (!A & R) | (B[i] & !(A ^ R));

        diff |= (D << i);
    }

    reg.A = diff;
    reg.flags(diff == 0, 1, B[4], B[8]);
}

void CPU::AND(uint8_t r) {
    uint8_t out = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;

        out |= (A & R) << i;
    }
    reg.A = out;
    reg.flags(out == 0, 0, 1, 0);
}

void CPU::OR(uint8_t r) {
    uint8_t out = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;

        out |= (A | R) << i;
    }
    reg.A = out;
    reg.flags(out == 0, 0, 0, 0);
}

void CPU::XOR(uint8_t r) {
    uint8_t out = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;

        out |= (A ^ R) << i;
    }
    reg.A = out;
    reg.flags(out == 0, 0, 0, 0);
}
uint8_t CPU::read_reg(uint8_t r) {
    switch (r) {
        case 0: return reg.B;
        case 1: return reg.C;
        case 2: return reg.D;
        case 3: return reg.E;
        case 4: return reg.H;
        case 5: return reg.L;
        case 6: return ram.read(reg.HL());
        case 7: return reg.A;
    }
    return 0;
}

void CPU::write_reg(uint8_t r, uint8_t val) {
    switch (r) {
        case 0: reg.B = val; break;
        case 1: reg.C = val; break;
        case 2: reg.D = val; break;
        case 3: reg.E = val; break;
        case 4: reg.H = val; break;
        case 5: reg.L = val; break;
        case 6: ram.write(reg.HL(), val); break;
        case 7: reg.A = val; break;
    }
}

void CPU::step() {
    uint8_t opcode = ram.read(reg.PC);
    reg.PC += 1;

    // LD REG1, REG2/(HL)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        if (opcode == 0x76) return;
        write_reg((opcode >> 3) & 7, read_reg(opcode & 7));
        return;
    }

    switch (opcode) {

        // ============== LOAD INSTRUCIONS ==============

        case 0x02: // LD (BC), A
            ram.write(reg.BC(), reg.A);
            break;

        case 0x12: // LD (DE), A
            ram.write(reg.DE(), reg.A);
            break;

        case 0x22: // LD (HL+), A
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() + 1);
            break;

        case 0x32: // LD (HL-), A
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() - 1);
            break;
        
        case 0x06: // LD B, d8
            reg.B = ram.read(reg.PC);
            reg.PC += 1;
            break;
        
        case 0x16: // LD D, d8
            reg.D = ram.read(reg.PC);
            reg.PC += 1;
            break;
        
        case 0x26: // LD H, d8
            reg.H = ram.read(reg.PC);
            reg.PC += 1;
            break;
        
        case 0x36: // LD (HL), d8
            ram.write(reg.HL(), ram.read(reg.PC));
            reg.PC += 1;
            break;
        
        case 0x0A: // LD A, (BC)
            reg.A = ram.read(reg.BC());
            break;
        
        case 0x1A: // LD A, (DE)
            reg.A = ram.read(reg.DE());
            break;
        
        case 0x2A: // LD A, (HL+)
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() + 1);
            break;
        
        case 0x3A: // LD A, (HL-)
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() - 1);
            break;

        case 0x0E: // LD C, d8
            reg.C = ram.read(reg.PC);
            reg.PC += 1;
            break;

        case 0x1E: // LD E, d8
            reg.E = ram.read(reg.PC);
            reg.PC += 1;
            break;

        case 0x2E: // LD L, d8
            reg.L = ram.read(reg.PC);
            reg.PC += 1;
            break;

        case 0x3E: // LD A, d8
            reg.A = ram.read(reg.PC);
            reg.PC += 1;
            break;

        case 0xE0: // LDH (a8), A
            ram.write(0xFF00 + ram.read(reg.PC), reg.A);
            reg.PC += 1;
            break;

        case 0xF0: // LDH A, (a8)
            reg.A = ram.read(0xFF00 + ram.read(reg.PC));
            reg.PC += 1;
            break;

        case 0xE2: // LD (C), A
            ram.write(0xFF00 + reg.C, reg.A);
            break;

        case 0xF2: // LD A, (C)
            reg.A = ram.read(0xFF00 + reg.C);
            break;
        
        case 0xEA: // LD (a16), A
            ram.write(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8), reg.A);
            reg.PC += 2;
            break;

        // ============== ADD INSTRUCTIONS ==============

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
            ADD(ram.read(reg.HL()));
            break;

        case 0x87: // ADD A, A
            ADD(reg.A);
            break;

        case 0xC6: // ADD A, d8
            ADD(ram.read(reg.PC));
            reg.PC += 1;
            break;
        
        // ============== ADC INSTRUCTIONS ==============

        case 0x88: // ADC A, B
            ADC(reg.B);
            break;

        case 0x89: // ADC A, C
            ADC(reg.C);
            break;

        case 0x8A: // ADC A, D
            ADC(reg.D);
            break;

        case 0x8B: // ADC A, E
            ADC(reg.E);
            break;

        case 0x8C: // ADC A, H
            ADC(reg.H);
            break;

        case 0x8D: // ADC A, L
            ADC(reg.L);
            break;

        case 0x8E: // ADC A, (HL)
            ADC(ram.read(reg.HL()));
            break;

        case 0x8F: // ADC A, A
            ADC(reg.A);
            break;

        case 0xCE: // ADC A, d8
            ADC(ram.read(reg.PC));
            reg.PC += 1;
            break;
        
        // ============== SUB INSTRUCTIONS ==============
        
        case 0x90: // SUB B
            SUB(reg.B);
            break;

        case 0x91: // SUB C
            SUB(reg.C);
            break;

        case 0x92: // SUB D
            SUB(reg.D);
            break;

        case 0x93: // SUB E
            SUB(reg.E);
            break;

        case 0x94: // SUB H
            SUB(reg.H);
            break;

        case 0x95: // SUB L
            SUB(reg.L);
            break;

        case 0x96: // SUB (HL)
            SUB(ram.read(reg.HL()));
            break;

        case 0x97: // SUB A
            SUB(reg.A);
            break;

        // ============== SBC INSTRUCTIONS ==============
        
        case 0x98: // SBC A, B
            SBC(reg.B);
            break;
        
        case 0x99: // SBC A, C
            SBC(reg.C);
            break;
        
        case 0x9A: // SBC A, D
            SBC(reg.D);
            break;
        
        case 0x9B: // SBC A, E
            SBC(reg.E);
            break;
        
        case 0x9C: // SBC A, H
            SBC(reg.H);
            break;
        
        case 0x9D: // SBC A, L
            SBC(reg.L);
            break;
        
        case 0x9E: // SBC A, (HL)
            SBC(ram.read(reg.HL()));
            break;
        
        case 0x9F: // SBC A, A
            SBC(reg.A);
            break;
            
        // ============== AND INSTRUCTIONS ==============

        
    }
}