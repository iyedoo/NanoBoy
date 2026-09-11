#include "../src/cpu/cpu.h"

#include <cstdint>
#include <iostream>

// THIS STRESS TEST WAS 100% AI-GENERATED, I HAVE A LIFE I SHOULDN'T BE DOING THIS SHIT

using namespace std;

int tests = 0;
int failed = 0;

void check(bool condition, const char* name) {
    ++tests;

    if (!condition) {
        ++failed;
        cout << "FAIL: " << name << '\n';
    }
}

uint8_t flags(bool z, bool n, bool h, bool c) {
    return (z << 7) | (n << 6) | (h << 5) | (c << 4);
}

void reset(CPU& cpu) {
    cpu.reg.A = 0;
    cpu.reg.F = 0;
    cpu.reg.B = 0;
    cpu.reg.C = 0;
    cpu.reg.D = 0;
    cpu.reg.E = 0;
    cpu.reg.H = 0;
    cpu.reg.L = 0;
    cpu.reg.PC = 0;
    cpu.reg.SP = 0;
}

// ==================== REGISTER LOADS ====================

void test_loads(CPU& cpu, RAM& ram) {

    // ==================== LD r, d8 ====================

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x06);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.B == v, "LD B,d8");
        check(cpu.reg.PC == 2, "LD B,d8 PC");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x0E);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.C == v, "LD C,d8");
        check(cpu.reg.PC == 2, "LD C,d8 PC");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x16);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.D == v, "LD D,d8");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x1E);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.E == v, "LD E,d8");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x26);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.H == v, "LD H,d8");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x2E);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.L == v, "LD L,d8");
    }

    for (int v = 0; v < 256; ++v) {
        reset(cpu);

        ram.write(0, 0x3E);
        ram.write(1, v);

        cpu.execute();

        check(cpu.reg.A == v, "LD A,d8");
    }

    // ==================== LD rr,d16 ====================

    for (int v = 0; v < 65536; ++v) {
        reset(cpu);

        ram.write(0, 0x01);
        ram.write(1, v & 0xFF);
        ram.write(2, v >> 8);

        cpu.execute();

        check(cpu.reg.BC() == v, "LD BC,d16");
        check(cpu.reg.PC == 3, "LD BC,d16 PC");
    }

    for (int v = 0; v < 65536; ++v) {
        reset(cpu);

        ram.write(0, 0x11);
        ram.write(1, v & 0xFF);
        ram.write(2, v >> 8);

        cpu.execute();

        check(cpu.reg.DE() == v, "LD DE,d16");
    }

    for (int v = 0; v < 65536; ++v) {
        reset(cpu);

        ram.write(0, 0x21);
        ram.write(1, v & 0xFF);
        ram.write(2, v >> 8);

        cpu.execute();

        check(cpu.reg.HL() == v, "LD HL,d16");
    }

    for (int v = 0; v < 65536; ++v) {
        reset(cpu);

        ram.write(0, 0x31);
        ram.write(1, v & 0xFF);
        ram.write(2, v >> 8);

        cpu.execute();

        check(cpu.reg.SP == v, "LD SP,d16");
    }
}

// ==================== LD r,r ====================

void test_register_transfers(CPU& cpu, RAM& ram) {

    for (int src = 0; src < 8; ++src) {
        for (int dst = 0; dst < 8; ++dst) {

            if (src == 6 && dst == 6)
                continue;

            reset(cpu);

            cpu.reg.B = 0x12;
            cpu.reg.C = 0x23;
            cpu.reg.D = 0x34;
            cpu.reg.E = 0x45;
            cpu.reg.H = 0x56;
            cpu.reg.L = 0x78;
            cpu.reg.A = 0x89;

            ram.write(0x5678, 0xAB);

            uint8_t values[8] = {
                cpu.reg.B,
                cpu.reg.C,
                cpu.reg.D,
                cpu.reg.E,
                cpu.reg.H,
                cpu.reg.L,
                ram.read(0x5678),
                cpu.reg.A
            };

            uint8_t opcode = 0x40 | (dst << 3) | src;

            ram.write(0, opcode);

            cpu.execute();

            uint8_t expected = values[src];
            uint8_t actual = 0;

            switch (dst) {
                case 0: actual = cpu.reg.B; break;
                case 1: actual = cpu.reg.C; break;
                case 2: actual = cpu.reg.D; break;
                case 3: actual = cpu.reg.E; break;
                case 4: actual = cpu.reg.H; break;
                case 5: actual = cpu.reg.L; break;
                case 6: actual = ram.read(0x5678); break;
                case 7: actual = cpu.reg.A; break;
            }

            check(actual == expected, "LD r,r");
        }
    }
}

// ==================== JP ====================

void test_jumps(CPU& cpu, RAM& ram) {

    // ==================== JP a16 ====================

    for (int target = 0; target < 65536; ++target) {
        reset(cpu);

        ram.write(0, 0xC3);
        ram.write(1, target & 0xFF);
        ram.write(2, target >> 8);

        cpu.execute();

        check(cpu.reg.PC == target, "JP a16");
    }

    // ==================== CONDITIONAL JP ====================

    const uint8_t opcodes[4] = {
        0xC2,
        0xCA,
        0xD2,
        0xDA
    };

    for (int opcode : opcodes) {
        for (int condition = 0; condition < 2; ++condition) {
            for (int target = 0; target < 65536; ++target) {

                reset(cpu);

                ram.write(0, opcode);
                ram.write(1, target & 0xFF);
                ram.write(2, target >> 8);

                if (opcode == 0xC2 || opcode == 0xCA)
                    cpu.reg.F = condition << 7;
                else
                    cpu.reg.F = condition << 4;

                cpu.execute();

                bool taken;

                if (opcode == 0xC2)
                    taken = !condition;
                else if (opcode == 0xCA)
                    taken = condition;
                else if (opcode == 0xD2)
                    taken = !condition;
                else
                    taken = condition;

                check(
                    cpu.reg.PC == (taken ? target : 3),
                    "conditional JP"
                );
            }
        }
    }

    // ==================== JP (HL) ====================

    for (int target = 0; target < 65536; ++target) {
        reset(cpu);

        cpu.reg.sHL(target);
        ram.write(0, 0xE9);

        cpu.execute();

        check(cpu.reg.PC == target, "JP (HL)");
    }

    // ==================== JR r8 ====================

    for (int offset = -128; offset <= 127; ++offset) {
        reset(cpu);

        ram.write(0, 0x18);
        ram.write(1, static_cast<uint8_t>(offset));

        cpu.execute();

        check(
            cpu.reg.PC == static_cast<uint16_t>(2 + offset),
            "JR r8"
        );
    }

    // ==================== CONDITIONAL JR ====================

    const uint8_t jr_opcodes[4] = {
        0x20,
        0x28,
        0x30,
        0x38
    };

    for (int opcode : jr_opcodes) {
        for (int condition = 0; condition < 2; ++condition) {
            for (int offset = -128; offset <= 127; ++offset) {

                reset(cpu);

                ram.write(0, opcode);
                ram.write(1, static_cast<uint8_t>(offset));

                if (opcode == 0x20 || opcode == 0x28)
                    cpu.reg.F = condition << 7;
                else
                    cpu.reg.F = condition << 4;

                cpu.execute();

                bool taken;

                if (opcode == 0x20)
                    taken = !condition;
                else if (opcode == 0x28)
                    taken = condition;
                else if (opcode == 0x30)
                    taken = !condition;
                else
                    taken = condition;

                uint16_t expected = taken
                    ? static_cast<uint16_t>(2 + offset)
                    : 2;

                check(cpu.reg.PC == expected, "conditional JR");
            }
        }
    }
}

// ==================== ROTATES ====================

void test_rotates(CPU& cpu, RAM& ram) {

    for (int a = 0; a < 256; ++a) {

        // RLCA

        reset(cpu);

        cpu.reg.A = a;
        ram.write(0, 0x07);

        cpu.execute();

        uint8_t cout = (a >> 7) & 1;
        uint8_t expected = (a << 1) | cout;

        check(cpu.reg.A == expected, "RLCA result");
        check(cpu.reg.F == flags(0, 0, 0, cout), "RLCA flags");

        // RRCA

        reset(cpu);

        cpu.reg.A = a;
        ram.write(0, 0x0F);

        cpu.execute();

        cout = a & 1;
        expected = (a >> 1) | (cout << 7);

        check(cpu.reg.A == expected, "RRCA result");
        check(cpu.reg.F == flags(0, 0, 0, cout), "RRCA flags");
    }

    for (int a = 0; a < 256; ++a) {
        for (int old_c = 0; old_c <= 1; ++old_c) {

            // RLA

            reset(cpu);

            cpu.reg.A = a;
            cpu.reg.F = old_c << 4;

            ram.write(0, 0x17);

            cpu.execute();

            uint8_t cout = (a >> 7) & 1;
            uint8_t expected = (a << 1) | old_c;

            check(cpu.reg.A == expected, "RLA result");
            check(cpu.reg.F == flags(0, 0, 0, cout), "RLA flags");

            // RRA

            reset(cpu);

            cpu.reg.A = a;
            cpu.reg.F = old_c << 4;

            ram.write(0, 0x1F);

            cpu.execute();

            cout = a & 1;
            expected = (a >> 1) | (old_c << 7);

            check(cpu.reg.A == expected, "RRA result");
            check(cpu.reg.F == flags(0, 0, 0, cout), "RRA flags");
        }
    }
}

// ==================== CONTROL ====================

void test_control(CPU& cpu, RAM& ram) {

    // SCF

    for (int f = 0; f < 256; ++f) {
        reset(cpu);

        cpu.reg.F = f;
        ram.write(0, 0x37);

        cpu.execute();

        check(
            cpu.reg.F == ((f & 0x80) | 0x10),
            "SCF"
        );
    }

    // CCF

    for (int f = 0; f < 256; ++f) {
        reset(cpu);

        cpu.reg.F = f;
        ram.write(0, 0x3F);

        cpu.execute();

        bool old_c = (f >> 4) & 1;

        check(
            cpu.reg.F == flags((f >> 7) & 1, 0, 0, !old_c),
            "CCF"
        );
    }

    // CPL

    for (int a = 0; a < 256; ++a) {
        for (int f = 0; f < 256; ++f) {

            reset(cpu);

            cpu.reg.A = a;
            cpu.reg.F = f;

            ram.write(0, 0x2F);

            cpu.execute();

            check(cpu.reg.A == static_cast<uint8_t>(~a), "CPL result");

            uint8_t expected =
                (f & 0x80) |
                0x60 |
                (f & 0x10);

            check(cpu.reg.F == expected, "CPL flags");
        }
    }
}

// ==================== INC / DEC ====================

void test_inc_dec(CPU& cpu, RAM& ram) {

    // INC8 instructions through execute()

    const uint8_t inc_opcodes[7] = {
        0x04,
        0x0C,
        0x14,
        0x1C,
        0x24,
        0x2C,
        0x3C
    };

    for (uint8_t opcode : inc_opcodes) {
        for (int value = 0; value < 256; ++value) {

            reset(cpu);

            cpu.reg.B = value;
            cpu.reg.C = value;
            cpu.reg.D = value;
            cpu.reg.E = value;
            cpu.reg.H = value;
            cpu.reg.L = value;
            cpu.reg.A = value;

            cpu.reg.F = 0x10;

            ram.write(0, opcode);

            cpu.execute();

            uint8_t expected = value + 1;

            uint8_t actual = 0;

            switch (opcode) {
                case 0x04: actual = cpu.reg.B; break;
                case 0x0C: actual = cpu.reg.C; break;
                case 0x14: actual = cpu.reg.D; break;
                case 0x1C: actual = cpu.reg.E; break;
                case 0x24: actual = cpu.reg.H; break;
                case 0x2C: actual = cpu.reg.L; break;
                case 0x3C: actual = cpu.reg.A; break;
            }

            check(actual == expected, "INC8 execute");
            check(
                cpu.reg.F == flags(expected == 0, 0, (value & 0xF) == 0xF, 1),
                "INC8 execute flags"
            );
        }
    }

    // ==================== INC16 ====================

    const uint8_t inc16_opcodes[4] = {
        0x03,
        0x13,
        0x23,
        0x33
    };

    for (uint8_t opcode : inc16_opcodes) {
        for (int value = 0; value < 65536; ++value) {

            reset(cpu);

            cpu.reg.sBC(value);
            cpu.reg.sDE(value);
            cpu.reg.sHL(value);
            cpu.reg.SP = value;

            cpu.reg.F = 0xF0;

            ram.write(0, opcode);

            cpu.execute();

            uint16_t expected = value + 1;
            uint16_t actual = 0;

            switch (opcode) {
                case 0x03: actual = cpu.reg.BC(); break;
                case 0x13: actual = cpu.reg.DE(); break;
                case 0x23: actual = cpu.reg.HL(); break;
                case 0x33: actual = cpu.reg.SP; break;
            }

            check(actual == expected, "INC16 execute");

            // INC rr must preserve ALL flags.
            check(cpu.reg.F == 0xF0, "INC16 preserves flags");
        }
    }
}

// ==================== MAIN ====================

int main() {

    RAM ram;
    CPU cpu(ram);

    test_loads(cpu, ram);
    test_register_transfers(cpu, ram);
    test_jumps(cpu, ram);
    test_rotates(cpu, ram);
    test_control(cpu, ram);
    test_inc_dec(cpu, ram);

    cout << '\n';
    cout << "CPU EXECUTION TESTS: " << tests << '\n';
    cout << "FAILED:              " << failed << '\n';

    if (failed == 0)
        cout << "ALL CPU TESTS PASSED :3\n";
    else
        cout << "CPU HAS BUGS 💀\n";

    return failed != 0;
}