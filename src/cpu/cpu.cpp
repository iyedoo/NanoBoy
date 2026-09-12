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
    bool Cout = (reg.A >> 7) & 1;
    reg.A = (reg.A << 1) | Cout;
    reg.flags(0, 0, 0, Cout);
}

void CPU::RLA() {
    bool C = (reg.F >> 4) & 1;
    bool Cout = (reg.A >> 7) & 1;
    reg.A = (reg.A << 1) | C;
    reg.flags(0, 0, 0, Cout);
}

void CPU::RRCA() {
    bool Cout = reg.A & 1;
    reg.A = (reg.A >> 1) | (Cout << 7);
    reg.flags(0, 0, 0, Cout);
}

void CPU::RRA() {
    bool C = (reg.F >> 4) & 1;
    bool Cout = reg.A & 1;
    reg.A = (reg.A >> 1) | (C << 7);
    reg.flags(0, 0, 0, Cout);
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

        case 0x00: break;                        // NOP        
        case 0x10: reg.PC += 1; STOP = 1; break; // STOP 0x00
        case 0x76: HALT = 1; break;              // HALT
        case 0xF3: break;                        // DI
        case 0xFB: break;                        // EI

        // ============== LOAD INSTRUCIONS ==============

        case 0x02: ram.write(reg.BC(), reg.A); break; // LD (BC), A
        case 0x12: ram.write(reg.DE(), reg.A); break; // LD (DE), A
        case 0x0A: reg.A = ram.read(reg.BC()); break; // LD A, (BC)
        case 0x1A: reg.A = ram.read(reg.DE()); break; // LD A, (DE)

        case 0x22: // LD (HL+), A
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() + 1);
            break;

        case 0x32: // LD (HL-), A
            ram.write(reg.HL(), reg.A);
            reg.sHL(reg.HL() - 1);
            break;
        
        case 0x2A: // LD A, (HL+)
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() + 1);
            break;
        
        case 0x3A: // LD A, (HL-)
            reg.A = ram.read(reg.HL());
            reg.sHL(reg.HL() - 1);
            break;
        
        case 0x06: reg.B = ram.read(reg.PC++); break;              // LD B, d8
        case 0x16: reg.D = ram.read(reg.PC++); break;              // LD D, d8
        case 0x26: reg.H = ram.read(reg.PC++); break;              // LD H, d8
        case 0x36: ram.write(reg.HL(), ram.read(reg.PC++)); break; // LD (HL), d8
        case 0x0E: reg.C = ram.read(reg.PC++); break;              // LD C, d8
        case 0x1E: reg.E = ram.read(reg.PC++); break;              // LD E, d8
        case 0x2E: reg.L = ram.read(reg.PC++); break;              // LD L, d8
        case 0x3E: reg.A = ram.read(reg.PC++); break;              // LD A, d8

        case 0xE0: ram.write(0xFF00 + ram.read(reg.PC++), reg.A); break; // LDH (a8), A
        case 0xF0: reg.A = ram.read(0xFF00 + ram.read(reg.PC++)); break; // LDH A, (a8)

        case 0xE2: ram.write(0xFF00 + reg.C, reg.A); break; // LD (C), A
        case 0xF2: reg.A = ram.read(0xFF00 + reg.C); break; // LD A, (C)
        
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
            
        case 0x08: { // LD (a16), SP
            uint16_t addr = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);

            ram.write(addr, reg.SP & 0xFF);
            ram.write(addr + 1, reg.SP >> 8);

            reg.PC += 2;
            break;
        }
            
        case 0xF8: { // LD HL, SP+r8
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
        case 0xF9: reg.SP = reg.HL(); break; // LD SP, HL

        // ============== ADD INSTRUCTIONS ==============

        case 0x80: ADD(reg.B); break;              // ADD A, B
        case 0x81: ADD(reg.C); break;              // ADD A, C
        case 0x82: ADD(reg.D); break;              // ADD A, D
        case 0x83: ADD(reg.E); break;              // ADD A, E
        case 0x84: ADD(reg.H); break;              // ADD A, H
        case 0x85: ADD(reg.L); break;              // ADD A, L
        case 0x86: ADD(ram.read(reg.HL())); break; // ADD A, (HL)
        case 0x87: ADD(reg.A); break;              // ADD A, A
        case 0xC6: ADD(ram.read(reg.PC++)); break; // ADD A, d8
        
        case 0xE8: { // ADD SP, r8
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
            
        case 0x09: ADD16(reg.BC()); break;
        case 0x19: ADD16(reg.DE()); break;
        case 0x29: ADD16(reg.HL()); break;
        case 0x39: ADD16(reg.SP); break;

        // ============== ADC INSTRUCTIONS ==============

        case 0x88: ADC(reg.B); break;              // ADC A, B
        case 0x89: ADC(reg.C); break;              // ADC A, C
        case 0x8A: ADC(reg.D); break;              // ADC A, D
        case 0x8B: ADC(reg.E); break;              // ADC A, E
        case 0x8C: ADC(reg.H); break;              // ADC A, H
        case 0x8D: ADC(reg.L); break;              // ADC A, L
        case 0x8E: ADC(ram.read(reg.HL())); break; // ADC A, (HL)
        case 0x8F: ADC(reg.A); break;              // ADC A, A
        case 0xCE: ADC(ram.read(reg.PC++)); break; // ADC A, d8
        
        // ============== SUB INSTRUCTIONS ==============
        
        case 0x90: SUB(reg.B); break;              // SUB B
        case 0x91: SUB(reg.C); break;              // SUB C
        case 0x92: SUB(reg.D); break;              // SUB D
        case 0x93: SUB(reg.E); break;              // SUB E
        case 0x94: SUB(reg.H); break;              // SUB H
        case 0x95: SUB(reg.L); break;              // SUB L
        case 0x96: SUB(ram.read(reg.HL())); break; // SUB (HL)
        case 0x97: SUB(reg.A); break;              // SUB A

        // ============== SBC INSTRUCTIONS ==============
        
        case 0x98: SBC(reg.B); break;              // SBC A, B
        case 0x99: SBC(reg.C); break;              // SBC A, C
        case 0x9A: SBC(reg.D); break;              // SBC A, D
        case 0x9B: SBC(reg.E); break;              // SBC A, E
        case 0x9C: SBC(reg.H); break;              // SBC A, H
        case 0x9D: SBC(reg.L); break;              // SBC A, L
        case 0x9E: SBC(ram.read(reg.HL())); break; // SBC A, (HL)
        case 0x9F: SBC(reg.A); break;              // SBC A, A

        // ============== AND INSTRUCTIONS ==============

        case 0xA0: AND(reg.B); break;              // AND B
        case 0xA1: AND(reg.C); break;              // AND C
        case 0xA2: AND(reg.D); break;              // AND D
        case 0xA3: AND(reg.E); break;              // AND E
        case 0xA4: AND(reg.H); break;              // AND H
        case 0xA5: AND(reg.L); break;              // AND L
        case 0xA6: AND(ram.read(reg.HL())); break; // AND (HL)
        case 0xA7: AND(reg.A); break;              // AND A
        case 0xE6: AND(ram.read(reg.PC++)); break; // AND d8

        // ============== XOR INSTRUCTIONS ==============

        case 0xA8: XOR(reg.B); break;              // XOR B
        case 0xA9: XOR(reg.C); break;              // XOR C
        case 0xAA: XOR(reg.D); break;              // XOR D
        case 0xAB: XOR(reg.E); break;              // XOR E
        case 0xAC: XOR(reg.H); break;              // XOR H
        case 0xAD: XOR(reg.L); break;              // XOR L
        case 0xAE: XOR(ram.read(reg.HL())); break; // XOR (HL)
        case 0xAF: XOR(reg.A); break;              // XOR A
        case 0xEE: XOR(ram.read(reg.PC++)); break; // XOR d8
            

        // ============== OR INSTRUCTIONS ==============

        case 0xB0: OR(reg.B); break;              // OR B
        case 0xB1: OR(reg.C); break;              // OR C
        case 0xB2: OR(reg.D); break;              // OR D
        case 0xB3: OR(reg.E); break;              // OR E
        case 0xB4: OR(reg.H); break;              // OR H
        case 0xB5: OR(reg.L); break;              // OR L
        case 0xB6: OR(ram.read(reg.HL())); break; // OR (HL)
        case 0xB7: OR(reg.A); break;              // OR A
        case 0xF6: OR(ram.read(reg.PC++)); break; // OR d8

        // ============== CP INSTRUCTIONS ==============

        case 0xB8: CP(reg.B); break;              // CP B
        case 0xB9: CP(reg.C); break;              // CP C
        case 0xBA: CP(reg.D); break;              // CP D
        case 0xBB: CP(reg.E); break;              // CP E
        case 0xBC: CP(reg.H); break;              // CP H
        case 0xBD: CP(reg.L); break;              // CP L
        case 0xBE: CP(ram.read(reg.HL())); break; // CP (HL)
        case 0xBF: CP(reg.A); break;              // CP A
        case 0xFE: CP(ram.read(reg.PC++)); break; // CP d8

        // ============== INC INSTRUCTIONS ==============

        case 0x04: reg.B = INC8(reg.B); break; // INC B
        case 0x14: reg.D = INC8(reg.D); break; // INC D
        case 0x24: reg.H = INC8(reg.H); break; // INC H        
        case 0x0C: reg.C = INC8(reg.C); break; // INC C
        case 0x1C: reg.E = INC8(reg.E); break; // INC E
        case 0x2C: reg.L = INC8(reg.L); break; // INC L
        case 0x3C: reg.A = INC8(reg.A); break; // INC A
        
        case 0x34: ram.write(reg.HL(), INC8(ram.read(reg.HL()))); break; // INC (HL)
        
        case 0x03: reg.sBC(INC16(reg.BC())); break; // INC BC
        case 0x13: reg.sDE(INC16(reg.DE())); break; // INC DE
        case 0x23: reg.sHL(INC16(reg.HL())); break; // INC HL
        case 0x33: reg.SP = INC16(reg.SP); break;   // INC SP

        // ============== DEC INSTRUCTIONS ==============

        case 0x05: reg.B = DEC8(reg.B); break;                           // DEC B
        case 0x0D: reg.C = DEC8(reg.C); break;                           // DEC C
        case 0x15: reg.D = DEC8(reg.D); break;                           // DEC D
        case 0x1D: reg.E = DEC8(reg.E); break;                           // DEC E
        case 0x25: reg.H = DEC8(reg.H); break;                           // DEC H
        case 0x2D: reg.L = DEC8(reg.L); break;                           // DEC L
        case 0x35: ram.write(reg.HL(), DEC8(ram.read(reg.HL()))); break; // DEC (HL)
        case 0x3D: reg.A = DEC8(reg.A); break;                           // DEC A

        case 0x0B: reg.sBC(DEC16(reg.BC())); break; // DEC BC
        case 0x1B: reg.sDE(DEC16(reg.DE())); break; // DEC DE
        case 0x2B: reg.sHL(DEC16(reg.HL())); break; // DEC HL
        case 0x3B: reg.SP = DEC16(reg.SP); break;   // DEC SP

        // ============== ROTATE INSTRUCTIONS ==============
        
        case 0x07: RLCA(); break;
        case 0x17: RLA();  break;
        case 0x0F: RRCA(); break;
        case 0x1F: RRA();  break;
        
        // ============== CONTROL INSTRUCTIONS ==============
        
        case 0x27: DAA(); break;
        case 0x37: SCF(); break;
        case 0x2F: CPL(); break;
        case 0x3F: CCF(); break;
    
        // ============== JUMP INSTRUCTIONS ==============

        case 0xC3: reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); break; // JP a16
        case 0xE9: reg.PC = reg.HL(); break; // JP (HL)
        
        case 0xC2: reg.PC = ((reg.F >> 7) & 1) ? reg.PC + 2 : ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); break; // JP NZ, a16
        case 0xCA: reg.PC = ((reg.F >> 7) & 1) ? ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8) : reg.PC + 2; break; // JP Z, a16
        case 0xD2: reg.PC = ((reg.F >> 4) & 1) ? reg.PC + 2 : ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8); break; // JP NC, a16
        case 0xDA: reg.PC = ((reg.F >> 4) & 1) ? ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8) : reg.PC + 2; break; // JP C, a16

        // I'm sorry to anyone reading this gibberish i just had to comrpess the file a bit...

        case 0x18: reg.PC += static_cast<int8_t>(ram.read(reg.PC)) + 1; break; // JR r8
        case 0x20: reg.PC += ((reg.F >> 7) & 1) ? 1 : static_cast<int8_t>(ram.read(reg.PC)) + 1; break; // JR NZ, r8
        case 0x28: reg.PC += ((reg.F >> 7) & 1) ? static_cast<int8_t>(ram.read(reg.PC)) + 1 : 1; break; // JR Z, r8
        case 0x30: reg.PC += ((reg.F >> 4) & 1) ? 1 : static_cast<int8_t>(ram.read(reg.PC)) + 1; break; // JR NC, r8
        case 0x38: reg.PC += ((reg.F >> 4) & 1) ? static_cast<int8_t>(ram.read(reg.PC)) + 1 : 1; break; // JR C, r8

        // ============== PUSH/POP INSTRUCTIONS ==============
        
        case 0xC5: // PUSH BC
            ram.write(reg.SP--, reg.B);
            ram.write(reg.SP--, reg.C);
            break;
        
        case 0xD5: // PUSH DE
            ram.write(reg.SP--, reg.D);
            ram.write(reg.SP--, reg.E);
            break;
        
        case 0xE5: // PUSH HL
            ram.write(reg.SP--, reg.H);
            ram.write(reg.SP--, reg.L);
            break;
        
        case 0xF5: // PUSH AF
            ram.write(reg.SP--, reg.A);
            ram.write(reg.SP--, reg.F);
            break;

        case 0xC1: // POP BC
            reg.C = ram.read(reg.SP++);
            reg.B = ram.read(reg.SP++);
            break;

        case 0xD1: // POP DE
            reg.E = ram.read(reg.SP++);
            reg.D = ram.read(reg.SP++);
            break;

        case 0xE1: // POP HL
            reg.L = ram.read(reg.SP++);
            reg.H = ram.read(reg.SP++);
            break;

        case 0xF1: // POP AF
            reg.F = ram.read(reg.SP++) & 0xF0;
            reg.A = ram.read(reg.SP++);
            break;

        // ============== CALL/RET INSTRUCTIONS ==============

        case 0xCD: // CALL nn
            ram.write(reg.SP--, ((reg.PC + 2) >> 8) & 0x00FF);
            ram.write(reg.SP--, (reg.PC + 2) & 0x00FF);
            reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            break;
        
        case 0xC4: // CALL NZ, nn
            if (!(reg.F & 0x80)) {
                ram.write(reg.SP--, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(reg.SP--, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            }
            else reg.PC += 2;
            break;

        case 0xCC: // CALL Z, nn
            if (reg.F & 0x80) {
                ram.write(reg.SP--, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(reg.SP--, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            }
            else reg.PC += 2;
            break;

        case 0xD4: // CALL NC, nn
            if (!(reg.F & 0x10)) {
                ram.write(reg.SP--, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(reg.SP--, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            }
            else reg.PC += 2;
            break;

        case 0xDC: // CALL C, nn
            if (reg.F & 0x10) {
                ram.write(reg.SP--, ((reg.PC + 2) >> 8) & 0xFF);
                ram.write(reg.SP--, (reg.PC + 2) & 0xFF);
                reg.PC = ram.read(reg.PC) | (ram.read(reg.PC + 1) << 8);
            }
            else reg.PC += 2;
            break;

    }
}