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

    bool Z = reg.F & 0b10000000;
    bool H = reg.F & 0b00100000;
    bool C = reg.F & 0b00010000;

    reg.flags(Z, 0, H, C);

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

    bool Z = reg.F & 0b10000000;
    bool H = reg.F & 0b00100000;
    bool C = reg.F & 0b00010000;

    reg.flags(Z, 1, H, C);

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

void CPU::execute() {
    uint8_t opcode = ram.read(reg.PC);
    reg.PC += 1;

    // LD REG1, REG2/(HL)
    if (opcode >= 0x40 && opcode <= 0x7F) {
        if (opcode == 0x76) return;
        write_reg((opcode >> 3) & 7, read_reg(opcode & 7));
        return;
    }

    switch (opcode) {

        case 0x00: // NOP
            break;
        
        case 0x10: // STOP 0
            break;
        
        case 0x76: // HALT
            break;
        
        

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
        
        case 0xFA: // LD A, (a16)
            reg.A = ram.read(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x01: // LD BC, d16
            reg.sBC(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x11: // LD DE, d16
            reg.sDE(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x21: // LD HL, d16
            reg.sHL(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8));
            reg.PC += 2;
            break;
        
        case 0x31: // LD SP, d16
            reg.SP = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            reg.PC += 2;
            break;
        
        case 0x08: // LD (a16), SP
            ram.write(ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8), reg.SP);
            reg.PC += 2;
            break;
        
        case 0xF8: // LD HL, SP+r8
            uint8_t r8 = ram.read(reg.PC);
            reg.PC += 1;

            bool H = (reg.SP & 0xF) + (r8 & 0xF) > 0xF;
            bool C = (reg.SP & 0xFF) + (r8 & 0xFF) > 0xF;

            reg.F = (H << 5) | (C << 4);

            reg.sHL(reg.SP + r8);
            break;

        case 0xF9: // LD SP, HL
            reg.SP = reg.HL();
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
        
        case 0xE8: // ADD SP, r8
            uint8_t r8 = ram.read(reg.PC);
            reg.PC += 1;

            bool H = (reg.SP & 0xF) + (r8 & 0xF) > 0xF;
            bool C = (reg.SP & 0xFF) + (r8 & 0xFF) > 0xF;

            reg.F = (H << 5) | (C << 4);

            reg.SP = reg.SP + r8;
            break;
        
        // ADD HL, rr
        // why am i doing this to myself
        // i could be talking to a girl rn instead of ts

        case 0x09:
            ADD16(reg.BC());
            break;
        case 0x19:
            ADD16(reg.DE());
            break;
        case 0x29:
            ADD16(reg.HL());
            break;
        case 0x39:
            ADD16(reg.SP);
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

        case 0xA0: // AND B
            AND(reg.B);
            break;

        case 0xA1: // AND C
            AND(reg.C);
            break;

        case 0xA2: // AND D
            AND(reg.D);
            break;

        case 0xA3: // AND E
            AND(reg.E);
            break;

        case 0xA4: // AND H
            AND(reg.H);
            break;

        case 0xA5: // AND L
            AND(reg.L);
            break;

        case 0xA6: // AND (HL)
            AND(ram.read(reg.HL()));
            break;

        case 0xA7: // AND A
            AND(reg.A);
            break;

        case 0xE6: // AND d8
            AND(ram.read(reg.PC));
            reg.PC += 1;
            break;

        // ============== XOR INSTRUCTIONS ==============

        case 0xA8: // XOR B
            XOR(reg.B);
            break;

        case 0xA9: // XOR C
            XOR(reg.C);
            break;

        case 0xAA: // XOR D
            XOR(reg.D);
            break;

        case 0xAB: // XOR E
            XOR(reg.E);
            break;

        case 0xAC: // XOR H
            XOR(reg.H);
            break;

        case 0xAD: // XOR L
            XOR(reg.L);
            break;

        case 0xAE: // XOR (HL)
            XOR(ram.read(reg.HL()));
            break;

        case 0xAF: // XOR A
            XOR(reg.A);
            break;

        case 0xEE: // XOR d8
            XOR(ram.read(reg.PC));
            reg.PC += 1;
            break;

        // ============== OR INSTRUCTIONS ==============

        case 0xB0: // OR B
            OR(reg.B);
            break;

        case 0xB1: // OR C
            OR(reg.C);
            break;

        case 0xB2: // OR D
            OR(reg.D);
            break;

        case 0xB3: // OR E
            OR(reg.E);
            break;

        case 0xB4: // OR H
            OR(reg.H);
            break;

        case 0xB5: // OR L
            OR(reg.L);
            break;

        case 0xB6: // OR (HL)
            OR(ram.read(reg.HL()));
            break;

        case 0xB7: // OR A
            OR(reg.A);
            break;

        case 0xF6: // OR d8
            OR(ram.read(reg.PC));
            reg.PC += 1;
            break;

        // ============== CP INSTRUCTIONS ==============

        case 0xB8: // CP B
            CP(reg.B);
            break;

        case 0xB9: // CP C
            CP(reg.C);
            break;

        case 0xBA: // CP D
            CP(reg.D);
            break;

        case 0xBB: // CP E
            CP(reg.E);
            break;

        case 0xBC: // CP H
            CP(reg.H);
            break;

        case 0xBD: // CP L
            CP(reg.L);
            break;

        case 0xBE: // CP (HL)
            CP(ram.read(reg.HL()));
            break;

        case 0xBF: // CP A
            CP(reg.A);
            break;

        case 0xFE: // CP d8
            CP(ram.read(reg.PC));
            reg.PC += 1;
            break;

        // ============== INC INSTRUCTIONS ==============

        case 0x04: // INC B
            reg.B = INC8(reg.B);
            break;

        case 0x0C: // INC C
            reg.C = INC8(reg.C);
            break;

        case 0x14: // INC D
            reg.D = INC8(reg.D);
            break;

        case 0x1C: // INC E
            reg.E = INC8(reg.E);
            break;

        case 0x24: // INC H
            reg.H = INC8(reg.H);
            break;

        case 0x2C: // INC L
            reg.L = INC8(reg.L);
            break;

        case 0x34: // INC (HL)
            ram.write(reg.HL(), INC8(ram.read(reg.HL())));
            break;

        case 0x3C: // INC A
            reg.A = INC8(reg.A);
            break;

        case 0x03: // INC BC
            reg.sBC(INC16(reg.BC()));
            break;

        case 0x13: // INC DE
            reg.sDE(INC16(reg.DE()));
            break;

        case 0x23: // INC HL
            reg.sHL(INC16(reg.HL()));
            break;

        case 0x33: // INC SP
            reg.SP = INC16(reg.SP);
            break;


        // ============== DEC INSTRUCTIONS ==============

        case 0x05: // DEC B
            reg.B = DEC8(reg.B);
            break;

        case 0x0D: // DEC C
            reg.C = DEC8(reg.C);
            break;

        case 0x15: // DEC D
            reg.D = DEC8(reg.D);
            break;

        case 0x1D: // DEC E
            reg.E = DEC8(reg.E);
            break;

        case 0x25: // DEC H
            reg.H = DEC8(reg.H);
            break;

        case 0x2D: // DEC L
            reg.L = DEC8(reg.L);
            break;

        case 0x35: // DEC (HL)
            ram.write(reg.HL(), DEC8(ram.read(reg.HL())));
            break;

        case 0x3D: // DEC A
            reg.A = DEC8(reg.A);
            break;

        case 0x0B: // DEC BC
            reg.sBC(DEC16(reg.BC()));
            break;

        case 0x1B: // DEC DE
            reg.sDE(DEC16(reg.DE()));
            break;

        case 0x2B: // DEC HL
            reg.sHL(DEC16(reg.HL()));
            break;

        case 0x3B: // DEC SP
            reg.SP = DEC16(reg.SP);
            break;
    }
}