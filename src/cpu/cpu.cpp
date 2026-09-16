#include "cpu.h"
#include <iostream>
#include <iomanip>

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

    HALT = 0;
    STOP = 0;
    IME = 0, enable = 0;

    clock = 0;
}

void CPU::ADD(uint8_t r) {
    bool c[9];

    uint8_t sum = 0;

    c[0] = 0;
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;
        
        bool S = A ^ R ^ c[i];
        c[i + 1] = (A & R) | (c[i] & (A ^ R));

        sum |= (S << i);
    }

    reg.A = sum;
    reg.flags(sum == 0, 0, c[4], c[8]);
}

void CPU::ADC(uint8_t r) {
    bool c[9];

    uint8_t sum = 0;

    c[0] = (reg.F & 0b00010000);
    for (int i = 0; i < 8; ++i) {
        bool A = (reg.A >> i) & 1;
        bool R = (r >> i) & 1;
        
        bool S = A ^ R ^ c[i];
        c[i + 1] = (A & R) | (c[i] & (A ^ R));

        sum |= (S << i);
    }

    reg.A = sum;
    reg.flags(sum == 0, 0, c[4], c[8]);
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

void CPU::CP(uint8_t r) {
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

    reg.flags(diff == 0, 1, B[4], B[8]);
}

uint8_t CPU::INC8(uint8_t r) {
    bool c[9];

    uint8_t sum = 0;

    c[0] = 0;
    for (int i = 0; i < 8; ++i) {
        bool R = (r >> i) & 1;
        bool V = (0b00000001 >> i) & 1;

        bool S = R ^ V ^ c[i];
        c[i + 1] = (R & V) | (c[i] & (R ^ V));

        sum |= (S << i);
    }

    bool C = reg.F & 0b00010000;
    reg.flags(sum == 0, 0, c[4], C);

    return sum;
}

uint8_t CPU::DEC8(uint8_t r) {
    bool B[9];

    uint8_t diff = 0;

    B[0] = 0;
    for (int i = 0; i < 8; ++i) {
        bool R = (r >> i) & 1;
        bool V = (0b00000001 >> i) & 1;

        bool D = R ^ V ^ B[i];
        B[i + 1] = (!R & V) | (B[i] & !(R ^ V));

        diff |= (D << i);
    }

    bool C = reg.F & 0b00010000;
    reg.flags(diff == 0, 1, B[4], C);

    return diff;
}

uint16_t CPU::INC16(uint16_t r) {
    bool c[17];

    uint16_t sum = 0;

    c[0] = 0;
    for (int i = 0; i < 16; ++i) {
        bool R = (r >> i) & 1;
        bool V = (0b0000000000000001 >> i) & 1;

        bool S = R ^ V ^ c[i];
        c[i + 1] = (R & V) | (c[i] & (R ^ V));

        sum |= (S << i);
    }

    return sum;
}

uint16_t CPU::DEC16(uint16_t r) {
    bool B[17];

    uint16_t diff = 0;

    B[0] = 0;
    for (int i = 0; i < 16; ++i) {
        bool R = (r >> i) & 1;
        bool V = (0b0000000000000001 >> i) & 1;

        bool D = R ^ V ^ B[i];
        B[i + 1] = (!R & V) | (B[i] & !(R ^ V));

        diff |= (D << i);
    }

    return diff;
}

void CPU::ADD16(uint16_t rr) {
    bool c[17];

    uint16_t sum = 0;

    c[0] = 0;
    for (int i = 0; i < 16; ++i) {
        bool HL = (reg.HL() >> i) & 1;
        bool R = (rr >> i) & 1;
        
        bool S = HL ^ R ^ c[i];
        c[i + 1] = (HL & R) | (c[i] & (HL ^ R));

        sum |= (S << i);
    }

    reg.sHL(sum);
    reg.flags(reg.F & 0x80, 0, c[12], c[16]);   
}

void CPU::DAA() {
    uint8_t adj = 0;
    bool C = reg.F & 0x10;

    if (!(reg.F & 0x40)) {
        if ((reg.F & 0x20) || (reg.A & 0x0F) > 9) adj |= 0x06;
        if (C || reg.A > 0x99) adj |= 0x60, C = 1;
        reg.A += adj;
    }
    else {
        if (reg.F & 0x20) adj |= 0x06;
        if (C) adj |= 0x60;
        reg.A -= adj;
    }

    reg.flags(reg.A == 0, reg.F & 0x40, 0, C);
}

void CPU::SCF() { reg.F = (reg.F & 0x80) | 0x10; }

void CPU::CPL() {
    reg.A = ~reg.A;
    reg.F = (reg.F & 0x90) | 0x60;
}

void CPU::CCF() { reg.flags((reg.F >> 7) & 1, 0, 0, !((reg.F >> 4) & 1)); }

void CPU::RLCA() {
    bool C = (reg.A >> 7) & 1;
    reg.A = (reg.A << 1) | C;
    reg.flags(0, 0, 0, C);
}

void CPU::RLA() {
    bool C = (reg.F >> 4) & 1;
    bool Cout = (reg.A >> 7) & 1;
    reg.A = (reg.A << 1) | C;
    reg.flags(0, 0, 0, Cout);
}

void CPU::RRCA() {
    bool C = reg.A & 1;
    reg.A = (reg.A >> 1) | (C << 7);
    reg.flags(0, 0, 0, C);
}

void CPU::RRA() {
    bool C = (reg.F >> 4) & 1;
    bool Cout = reg.A & 1;
    reg.A = (reg.A >> 1) | (C << 7);
    reg.flags(0, 0, 0, Cout);
}

void CPU::RLC(uint8_t& x) {
    bool C = (x >> 7) & 1;
    x = (x << 1) | C;
    reg.flags(x == 0, 0, 0, C);
}

void CPU::RL(uint8_t& x) {
    bool C = (reg.F >> 4) & 1;
    bool Cout = (x >> 7) & 1;
    x = (x << 1) | C;
    reg.flags(x == 0, 0, 0, Cout);
}

void CPU::RRC(uint8_t& x) {
    bool C = x & 1;
    x = (x >> 1) | (C << 7);
    reg.flags(x == 0, 0, 0, C);
}

void CPU::RR(uint8_t& x) {
    bool C = (reg.F >> 4) & 1;
    bool Cout = x & 1;
    x = (x >> 1) | (C << 7);
    reg.flags(x == 0, 0, 0, Cout);
}

void CPU::SLA(uint8_t& x) {
    bool C = (x >> 7) & 1;
    x <<= 1;
    reg.flags(x == 0, 0, 0, C);
}

void CPU::SRA(uint8_t& x) {
    bool C = x & 1;
    uint8_t MSB = x & 0x80;
    x = (x >> 1) | MSB;
    reg.flags(x == 0, 0, 0, C);
}

void CPU::SRL(uint8_t& x) {
    bool C = x & 1;
    x >>= 1;
    reg.flags(x == 0, 0, 0, C);
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

bool CPU::interrupts() {
    uint8_t IE = ram.read(0xFFFF);
    uint8_t IF = ram.read(0xFF0F);
    bool pending = IE & IF & 0x1F;

    if (!pending) return 0;
    if (HALT) HALT = 0;
    if (!IME) return 0;
    IME = 0;

    for (int i = 0; i < 5; ++i) {
        if ((pending >> i) & 1) {
            IF &= ~(1 << i);
            ram.write(0xFF0F, IF);
            ram.write(--reg.SP, reg.PC >> 8);
            ram.write(--reg.SP, reg.PC & 0xFF);

            reg.PC = 0x0040 + i * 8;
            return 1;
        }
    }
    return 0;
}

void CPU::execute() {

    if (interrupts()) return;
    if (HALT) return;

    uint8_t opcode = ram.read(reg.PC);
    reg.PC += 1;

    // LD REG1, REG2/(HL)
    if (opcode >= 0x40 && opcode <= 0x7F && opcode != 0x76) {
        uint8_t dst = (opcode >> 3) & 7;
        uint8_t src = opcode & 7;

        clock += (dst == 6 || src == 6) ? 8 : 4;
        write_reg(dst, read_reg(src));
        return;
    }

    switch (opcode) {

        case 0x00: clock += 4; break;                        // NOP        
        case 0x10: clock += 4; reg.PC += 1; STOP = 1; break; // STOP 0x00
        case 0x76: clock += 4; HALT = 1; break;              // HALT
        case 0xF3: clock += 4; IME = 0, enable = 0; break;   // DI
        case 0xFB: clock += 4; enable = 2; break;            // EI
        case 0xD9: clock += 16; reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8); IME = 1; break;

        // ============== LOAD INSTRUCIONS ==============

        case 0x02: clock += 8; ram.write(reg.BC(), reg.A); break; // LD (BC), A
        case 0x12: clock += 8; ram.write(reg.DE(), reg.A); break; // LD (DE), A
        case 0x0A: clock += 8; reg.A = ram.read(reg.BC()); break; // LD A, (BC)
        case 0x1A: clock += 8; reg.A = ram.read(reg.DE()); break; // LD A, (DE)

        case 0x22: // LD (HL+), A
            clock += 8;
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() + 1);
            break;

        case 0x32: // LD (HL-), A
            clock += 8;
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() - 1);
            break;
        
        case 0x2A: // LD A, (HL+)
            clock += 8;
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() + 1);
            break;
        
        case 0x3A: // LD A, (HL-)
            clock += 8;
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() - 1);
            break;
        
        case 0x06: clock += 8; reg.B = ram.read(reg.PC++); break;              // LD B, d8
        case 0x16: clock += 8; reg.D = ram.read(reg.PC++); break;              // LD D, d8
        case 0x26: clock += 8; reg.H = ram.read(reg.PC++); break;              // LD H, d8
        case 0x36: clock += 12;ram.write(reg.HL(), ram.read(reg.PC++)); break; // LD (HL), d8
        case 0x0E: clock += 8; reg.C = ram.read(reg.PC++); break;              // LD C, d8
        case 0x1E: clock += 8; reg.E = ram.read(reg.PC++); break;              // LD E, d8
        case 0x2E: clock += 8; reg.L = ram.read(reg.PC++); break;              // LD L, d8
        case 0x3E: clock += 8; reg.A = ram.read(reg.PC++); break;              // LD A, d8

        case 0xE0: clock += 12; ram.write(0xFF00 + ram.read(reg.PC++), reg.A); break; // LDH (a8), A
        case 0xF0: clock += 12; reg.A = ram.read(0xFF00 + ram.read(reg.PC++)); break; // LDH A, (a8)

        case 0xE2: clock += 8; ram.write(0xFF00 + reg.C, reg.A); break; // LD (C), A
        case 0xF2: clock += 8; reg.A = ram.read(0xFF00 + reg.C); break; // LD A, (C)
        
        case 0xEA: // LD (a16), A
            clock += 16;
            ram.write(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8), reg.A);
            reg.PC += 2;
            break;
        
        case 0xFA: // LD A, (a16)
            clock += 16;
            reg.A = ram.read(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x01: // LD BC, d16
            clock += 12;
            reg.sBC(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x11: // LD DE, d16
            clock += 12;
            reg.sDE(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x21: // LD HL, d16
            clock += 12;
            reg.sHL(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x31: // LD SP, d16
            clock += 12;
            reg.SP = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            reg.PC += 2;
            break;
            
        case 0x08: { // LD (a16), SP
            clock += 20;
            uint16_t addr = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);

            ram.write(addr, reg.SP & 0xFF);
            ram.write(addr + 1, reg.SP >> 8);

            reg.PC += 2;
            break;
        }
            
        case 0xF8: { // LD HL, SP+r8
            clock += 12;
            uint8_t raw = ram.read(reg.PC);
            int8_t r8 = static_cast<int8_t>(raw);
            reg.PC += 1;
            
            bool H = (reg.SP & 0xF) + (raw & 0xF) > 0xF;
            bool C = (reg.SP & 0xFF) + (raw & 0xFF) > 0xFF;
            
            reg.F = (H << 5) | (C << 4);
            reg.sHL(static_cast<uint16_t>(reg.SP + r8));
            break;
        }
            
        // A bug here took exactly 1 hour, 37 minutes to fix
        case 0xF9: clock += 8; reg.SP = reg.HL(); break; // LD SP, HL

        // ============== ADD INSTRUCTIONS ==============

        case 0x80: clock += 4; ADD(reg.B); break;              // ADD A, B
        case 0x81: clock += 4; ADD(reg.C); break;              // ADD A, C
        case 0x82: clock += 4; ADD(reg.D); break;              // ADD A, D
        case 0x83: clock += 4; ADD(reg.E); break;              // ADD A, E
        case 0x84: clock += 4; ADD(reg.H); break;              // ADD A, H
        case 0x85: clock += 4; ADD(reg.L); break;              // ADD A, L
        case 0x86: clock += 8; ADD(ram.read(reg.HL())); break; // ADD A, (HL)
        case 0x87: clock += 4; ADD(reg.A); break;              // ADD A, A
        case 0xC6: clock += 8; ADD(ram.read(reg.PC++)); break; // ADD A, d8
        
        case 0xE8: { // ADD SP, r8
            clock += 16;
            uint8_t raw = ram.read(reg.PC);
            int8_t r8 = static_cast<int8_t>(raw);
            reg.PC += 1;

            bool H = (reg.SP & 0xF) + (raw & 0xF) > 0xF;
            bool C = (reg.SP & 0xFF) + (raw & 0xFF) > 0xFF;

            reg.F = (H << 5) | (C << 4);
            reg.SP = static_cast<uint16_t>(reg.SP + r8);
            break;
        }
        
        // why am i doing this to myself
        // i could be talking to a girl rn instead of ts
        
        // ADD HL, rr
            
        case 0x09: clock += 8; ADD16(reg.BC()); break;
        case 0x19: clock += 8; ADD16(reg.DE()); break;
        case 0x29: clock += 8; ADD16(reg.HL()); break;
        case 0x39: clock += 8; ADD16(reg.SP); break;

        // ============== ADC INSTRUCTIONS ==============

        case 0x88: clock += 4; ADC(reg.B); break;              // ADC A, B
        case 0x89: clock += 4; ADC(reg.C); break;              // ADC A, C
        case 0x8A: clock += 4; ADC(reg.D); break;              // ADC A, D
        case 0x8B: clock += 4; ADC(reg.E); break;              // ADC A, E
        case 0x8C: clock += 4; ADC(reg.H); break;              // ADC A, H
        case 0x8D: clock += 4; ADC(reg.L); break;              // ADC A, L
        case 0x8E: clock += 8; ADC(ram.read(reg.HL())); break; // ADC A, (HL)
        case 0x8F: clock += 4; ADC(reg.A); break;              // ADC A, A
        case 0xCE: clock += 8; ADC(ram.read(reg.PC++)); break; // ADC A, d8
        
        // ============== SUB INSTRUCTIONS ==============
        
        case 0x90: clock += 4; SUB(reg.B); break;              // SUB B
        case 0x91: clock += 4; SUB(reg.C); break;              // SUB C
        case 0x92: clock += 4; SUB(reg.D); break;              // SUB D
        case 0x93: clock += 4; SUB(reg.E); break;              // SUB E
        case 0x94: clock += 4; SUB(reg.H); break;              // SUB H
        case 0x95: clock += 4; SUB(reg.L); break;              // SUB L
        case 0x96: clock += 8; SUB(ram.read(reg.HL())); break; // SUB (HL)
        case 0x97: clock += 4; SUB(reg.A); break;              // SUB A
        case 0xD6: clock += 8; SUB(ram.read(reg.PC++)); break; // SUB d8
        
        // ============== SBC INSTRUCTIONS ==============
        
        case 0x98: clock += 4; SBC(reg.B); break;              // SBC A, B
        case 0x99: clock += 4; SBC(reg.C); break;              // SBC A, C
        case 0x9A: clock += 4; SBC(reg.D); break;              // SBC A, D
        case 0x9B: clock += 4; SBC(reg.E); break;              // SBC A, E
        case 0x9C: clock += 4; SBC(reg.H); break;              // SBC A, H
        case 0x9D: clock += 4; SBC(reg.L); break;              // SBC A, L
        case 0x9E: clock += 8; SBC(ram.read(reg.HL())); break; // SBC A, (HL)
        case 0x9F: clock += 4; SBC(reg.A); break;              // SBC A, A
        case 0xDE: clock += 8; SBC(ram.read(reg.PC++)); break; // SBC d8
        
        // ============== AND INSTRUCTIONS ==============

        case 0xA0: clock += 4; AND(reg.B); break;              // AND B
        case 0xA1: clock += 4; AND(reg.C); break;              // AND C
        case 0xA2: clock += 4; AND(reg.D); break;              // AND D
        case 0xA3: clock += 4; AND(reg.E); break;              // AND E
        case 0xA4: clock += 4; AND(reg.H); break;              // AND H
        case 0xA5: clock += 4; AND(reg.L); break;              // AND L
        case 0xA6: clock += 8; AND(ram.read(reg.HL())); break; // AND (HL)
        case 0xA7: clock += 4; AND(reg.A); break;              // AND A
        case 0xE6: clock += 8; AND(ram.read(reg.PC++)); break; // AND d8

        // ============== XOR INSTRUCTIONS ==============

        case 0xA8: clock += 4; XOR(reg.B); break;              // XOR B
        case 0xA9: clock += 4; XOR(reg.C); break;              // XOR C
        case 0xAA: clock += 4; XOR(reg.D); break;              // XOR D
        case 0xAB: clock += 4; XOR(reg.E); break;              // XOR E
        case 0xAC: clock += 4; XOR(reg.H); break;              // XOR H
        case 0xAD: clock += 4; XOR(reg.L); break;              // XOR L
        case 0xAE: clock += 8; XOR(ram.read(reg.HL())); break; // XOR (HL)
        case 0xAF: clock += 4; XOR(reg.A); break;              // XOR A
        case 0xEE: clock += 8; XOR(ram.read(reg.PC++)); break; // XOR d8
            

        // ============== OR INSTRUCTIONS ==============

        case 0xB0: clock += 4; OR(reg.B); break;              // OR B
        case 0xB1: clock += 4; OR(reg.C); break;              // OR C
        case 0xB2: clock += 4; OR(reg.D); break;              // OR D
        case 0xB3: clock += 4; OR(reg.E); break;              // OR E
        case 0xB4: clock += 4; OR(reg.H); break;              // OR H
        case 0xB5: clock += 4; OR(reg.L); break;              // OR L
        case 0xB6: clock += 8; OR(ram.read(reg.HL())); break; // OR (HL)
        case 0xB7: clock += 4; OR(reg.A); break;              // OR A
        case 0xF6: clock += 8; OR(ram.read(reg.PC++)); break; // OR d8

        // ============== CP INSTRUCTIONS ==============

        case 0xB8: clock += 4; CP(reg.B); break;              // CP B
        case 0xB9: clock += 4; CP(reg.C); break;              // CP C
        case 0xBA: clock += 4; CP(reg.D); break;              // CP D
        case 0xBB: clock += 4; CP(reg.E); break;              // CP E
        case 0xBC: clock += 4; CP(reg.H); break;              // CP H
        case 0xBD: clock += 4; CP(reg.L); break;              // CP L
        case 0xBE: clock += 8; CP(ram.read(reg.HL())); break; // CP (HL)
        case 0xBF: clock += 4; CP(reg.A); break;              // CP A
        case 0xFE: clock += 8; CP(ram.read(reg.PC++)); break; // CP d8

        // ============== INC INSTRUCTIONS ==============
        
        case 0x04: clock += 4; reg.B = INC8(reg.B); break;                           // INC B
        case 0x14: clock += 4; reg.D = INC8(reg.D); break;                           // INC D
        case 0x24: clock += 4; reg.H = INC8(reg.H); break;                           // INC H        
        case 0x34: clock += 8; ram.write(reg.HL(), INC8(ram.read(reg.HL()))); break; // INC (HL)
        case 0x0C: clock += 4; reg.C = INC8(reg.C); break;                           // INC C
        case 0x1C: clock += 4; reg.E = INC8(reg.E); break;                           // INC E
        case 0x2C: clock += 4; reg.L = INC8(reg.L); break;                           // INC L
        case 0x3C: clock += 4; reg.A = INC8(reg.A); break;                           // INC A
        
        case 0x03: clock += 8; reg.sBC(INC16(reg.BC())); break; // INC BC
        case 0x13: clock += 8; reg.sDE(INC16(reg.DE())); break; // INC DE
        case 0x23: clock += 8; reg.sHL(INC16(reg.HL())); break; // INC HL
        case 0x33: clock += 8; reg.SP = INC16(reg.SP); break;   // INC SP

        // ============== DEC INSTRUCTIONS ==============

        case 0x05: clock += 4; reg.B = DEC8(reg.B); break;                           // DEC B
        case 0x15: clock += 4; reg.D = DEC8(reg.D); break;                           // DEC D
        case 0x25: clock += 4; reg.H = DEC8(reg.H); break;                           // DEC H
        case 0x35: clock += 8; ram.write(reg.HL(), DEC8(ram.read(reg.HL()))); break; // DEC (HL)
        case 0x0D: clock += 4; reg.C = DEC8(reg.C); break;                           // DEC C
        case 0x1D: clock += 4; reg.E = DEC8(reg.E); break;                           // DEC E
        case 0x2D: clock += 4; reg.L = DEC8(reg.L); break;                           // DEC L
        case 0x3D: clock += 4; reg.A = DEC8(reg.A); break;                           // DEC A

        case 0x0B: clock += 8; reg.sBC(DEC16(reg.BC())); break; // DEC BC
        case 0x1B: clock += 8; reg.sDE(DEC16(reg.DE())); break; // DEC DE
        case 0x2B: clock += 8; reg.sHL(DEC16(reg.HL())); break; // DEC HL
        case 0x3B: clock += 8; reg.SP = DEC16(reg.SP); break;   // DEC SP

        // ============== ROTATE INSTRUCTIONS ==============
        
        case 0x07: clock += 4; RLCA(); break;
        case 0x17: clock += 4; RLA();  break;
        case 0x0F: clock += 4; RRCA(); break;
        case 0x1F: clock += 4; RRA();  break;
        
        // ============== CONTROL INSTRUCTIONS ==============
        
        case 0x27: clock += 4; DAA(); break;
        case 0x37: clock += 4; SCF(); break;
        case 0x2F: clock += 4; CPL(); break;
        case 0x3F: clock += 4; CCF(); break;
    
        // ============== JUMP INSTRUCTIONS ==============

        case 0xC3: clock += 16; reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); break; // JP a16
        case 0xE9: clock += 4; reg.PC = reg.HL(); break; // JP (HL)
        
        case 0xC2: reg.PC = ((reg.F >> 7) & 1) ? reg.PC + 2 : ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); clock += ((reg.F >> 7) & 1) ? 12 : 16; break; // JP NZ, a16
        case 0xCA: reg.PC = ((reg.F >> 7) & 1) ? ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8) : reg.PC + 2; clock += ((reg.F >> 7) & 1) ? 16 : 12; break; // JP Z, a16
        case 0xD2: reg.PC = ((reg.F >> 4) & 1) ? reg.PC + 2 : ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); clock += ((reg.F >> 4) & 1) ? 12 : 16; break; // JP NC, a16
        case 0xDA: reg.PC = ((reg.F >> 4) & 1) ? ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8) : reg.PC + 2; clock += ((reg.F >> 4) & 1) ? 16 : 12; break; // JP C, a16

        // I'm sorry to anyone reading this gibberish i just had to comrpess the file a bit...
        
        case 0x18: reg.PC += static_cast<int8_t>(ram.read(reg.PC)) + 1; clock = 12; break; // JR r8
        case 0x20: reg.PC += ((reg.F >> 7) & 1) ? 1 : static_cast<int8_t>(ram.read(reg.PC)) + 1; clock += ((reg.F >> 7) & 1) ? 8 : 12; break; // JR NZ, r8
        case 0x28: reg.PC += ((reg.F >> 7) & 1) ? static_cast<int8_t>(ram.read(reg.PC)) + 1 : 1; clock += ((reg.F >> 7) & 1) ? 12 : 8; break; // JR Z, r8
        case 0x30: reg.PC += ((reg.F >> 4) & 1) ? 1 : static_cast<int8_t>(ram.read(reg.PC)) + 1; clock += ((reg.F >> 4) & 1) ? 8 : 12; break; // JR NC, r8
        case 0x38: reg.PC += ((reg.F >> 4) & 1) ? static_cast<int8_t>(ram.read(reg.PC)) + 1 : 1; clock += ((reg.F >> 4) & 1) ? 12 : 8; break; // JR C, r8

        // ============== PUSH/POP INSTRUCTIONS ==============
        
        case 0xC5: // PUSH BC
            clock += 12;
            ram.write(--reg.SP, reg.B);
            ram.write(--reg.SP, reg.C);
            break;
        
        case 0xD5: // PUSH DE
            clock += 12;
            ram.write(--reg.SP, reg.D);
            ram.write(--reg.SP, reg.E);
            break;
        
        case 0xE5: // PUSH HL
            clock += 12;
            ram.write(--reg.SP, reg.H);
            ram.write(--reg.SP, reg.L);
            break;
        
        case 0xF5: // PUSH AF
            clock += 12;
            ram.write(--reg.SP, reg.A);
            ram.write(--reg.SP, reg.F);
            break;

        case 0xC1: // POP BC
            clock += 12;
            reg.C = ram.read(reg.SP++);
            reg.B = ram.read(reg.SP++);
            break;

        case 0xD1: // POP DE
            clock += 12;
            reg.E = ram.read(reg.SP++);
            reg.D = ram.read(reg.SP++);
            break;

        case 0xE1: // POP HL
            clock += 12;
            reg.L = ram.read(reg.SP++);
            reg.H = ram.read(reg.SP++);
            break;

        case 0xF1: // POP AF
            clock += 12;
            reg.F = ram.read(reg.SP++) & 0xF0;
            reg.A = ram.read(reg.SP++);
            break;

        // ============== CALL/RET/RST INSTRUCTIONS ==============

        case 0xCD: // CALL nn
            ram.write(--reg.SP, ((reg.PC + 2) >> 8) & 0x00FF);
            ram.write(--reg.SP, (reg.PC + 2) & 0x00FF);
            reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            clock += 24;
            break;
        
        case 0xC4: // CALL NZ, nn
            if (!(reg.F & 0x80)) {
                ram.write(--reg.SP, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(--reg.SP, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
                clock += 24;
            }
            else {
                reg.PC += 2;
                clock += 12;
            }
            break;

        case 0xCC: // CALL Z, nn
            if (reg.F & 0x80) {
                ram.write(--reg.SP, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(--reg.SP, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
                clock += 24;
            }
            else {
                reg.PC += 2;
                clock += 12;
            }
            break;

        case 0xD4: // CALL NC, nn
            if (!(reg.F & 0x10)) {
                ram.write(--reg.SP, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(--reg.SP, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
                clock += 24;
            }
            else {
                reg.PC += 2;
                clock += 12;
            }
            break;

        case 0xDC: // CALL C, nn
            if (reg.F & 0x10) {
                ram.write(--reg.SP, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(--reg.SP, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
                clock += 24;
            }
            else {
                reg.PC += 2;
                clock += 12;
            }
            break;
        
        case 0xC9: // RET
            reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8);
            clock += 16;
            break;

        case 0xC0: // RET NZ
            if (!(reg.F & 0x80)) {
                reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8);
                clock += 20;
            }
            else clock += 8;
            break;

        case 0xC8: // RET Z
            if (reg.F & 0x80) {
                reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8);
                clock += 20;
            }
            else clock += 8;
            break;

        case 0xD0: // RET NC
            if (!(reg.F & 0x10)) {
                reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8);
                clock += 20;
            }
            else clock += 8;
            break;

        case 0xD8: // RET C
            if (reg.F & 0x10) {
                reg.PC = ram.read(reg.SP++) | (ram.read(reg.SP++) << 8);
                clock += 20;
            }
            else clock += 8;
            break;
    
        // RST Family (I wish the family gets destroyed)
        case 0xC7:
        case 0xCF:
        case 0xD7:
        case 0xDF:
        case 0xE7:
        case 0xEF:
        case 0xF7:
        case 0xFF:
            ram.write(--reg.SP, (reg.PC >> 8) & 0xFF);
            ram.write(--reg.SP, reg.PC & 0xFF);
            reg.PC = opcode - 0xC7;
            clock += 16;
            break;

        // ================ CB PREFIX INTSTRUCTIONS ===================
        // HELL'S COMIIIIIIIIIING WITH ME (go check the song)

        case 0xCB: {
            uint8_t nxt = ram.read(reg.PC++);

            uint8_t b = (nxt >> 3) & 7;
            uint8_t r = nxt & 7;
            uint8_t x = read_reg(r);

            clock += r == 6 ? 16 : 8;

            if (nxt < 0x08) {      // RLC r
                RLC(x);
                write_reg(r, x);
            }
            else if (nxt < 0x10) { // RRC r
                RRC(x);
                write_reg(r, x);
            }
            else if (nxt < 0x18) { // RL r
                RL(x);
                write_reg(r, x);
            }
            else if (nxt < 0x20) { // RR r
                RR(x);
                write_reg(r, x);
            }
            else if (nxt < 0x28) { // SLA r
                SLA(x);
                write_reg(r, x);
            }
            else if (nxt < 0x30) { // SRA r
                SRA(x);
                write_reg(r, x);
            }
            else if (nxt < 0x38) { // SWAP r
                x = (x << 4) | (x >> 4);
                write_reg(r, x);
                reg.flags(x == 0, 0, 0, 0);
            }
            else if (nxt < 0x40) { // SRL r
                SRL(x);
                write_reg(r, x);
            }
            else if (nxt < 0x80) { // BIT b, r
                bool BIT = (x >> b) & 1;
                reg.flags(!BIT, 0, 1, (reg.F >> 4) & 1);
                clock = r == 6 ? 12 : 8;
            }
            else if (nxt < 0xC0) { // RES b, r
                write_reg(r, x & ~(1 << b));
            }
            else {                 // SET b, r
                write_reg(r, x | (1 << b));
            }

            break;
        }

        default:
            std::cerr << "Unknown opcode: 0x"
                    << std::hex << std::setw(2) << std::setfill('0') << (int)opcode
                    << " at PC=0x"
                    << std::setw(4) << (int)(reg.PC - 1)
                    << '\n';
            break;

    }
    if (enable) {
        --enable;
        if (!enable) IME = 1;
    }
}