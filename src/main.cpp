#include "../src/cpu/cpu.h"

#include <cstdint>
#include <iostream>

// THIS STRESS TEST WAS COMPLETELY VIBE-CODED

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

int main() {
    RAM ram;
    CPU cpu(ram);

    // ==================== ADD ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;
            cpu.reg.F = 0;

            cpu.ADD(r);

            uint16_t result = a + r;
            uint8_t expected = result & 0xFF;

            bool z = expected == 0;
            bool h = ((a & 0xF) + (r & 0xF)) > 0xF;
            bool c = result > 0xFF;

            check(cpu.reg.A == expected, "ADD result");
            check(cpu.reg.F == flags(z, 0, h, c), "ADD flags");
        }
    }

    // ==================== ADC ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            for (int old_c = 0; old_c <= 1; ++old_c) {
                cpu.reg.A = a;
                cpu.reg.F = old_c << 4;

                cpu.ADC(r);

                uint16_t result = a + r + old_c;
                uint8_t expected = result & 0xFF;

                bool z = expected == 0;
                bool h = ((a & 0xF) + (r & 0xF) + old_c) > 0xF;
                bool c = result > 0xFF;

                check(cpu.reg.A == expected, "ADC result");
                check(cpu.reg.F == flags(z, 0, h, c), "ADC flags");
            }
        }
    }

    // ==================== SUB ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;
            cpu.reg.F = 0;

            cpu.SUB(r);

            uint8_t expected = (a - r) & 0xFF;

            bool z = expected == 0;
            bool h = (a & 0xF) < (r & 0xF);
            bool c = a < r;

            check(cpu.reg.A == expected, "SUB result");
            check(cpu.reg.F == flags(z, 1, h, c), "SUB flags");
        }
    }

    // ==================== SBC ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            for (int old_c = 0; old_c <= 1; ++old_c) {
                cpu.reg.A = a;
                cpu.reg.F = old_c << 4;

                cpu.SBC(r);

                int result = a - r - old_c;
                uint8_t expected = result & 0xFF;

                bool z = expected == 0;
                bool h = (a & 0xF) < ((r & 0xF) + old_c);
                bool c = a < (r + old_c);

                check(cpu.reg.A == expected, "SBC result");
                check(cpu.reg.F == flags(z, 1, h, c), "SBC flags");
            }
        }
    }

    // ==================== AND ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;

            cpu.AND(r);

            uint8_t expected = a & r;

            check(cpu.reg.A == expected, "AND result");
            check(cpu.reg.F == flags(expected == 0, 0, 1, 0), "AND flags");
        }
    }

    // ==================== OR ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;

            cpu.OR(r);

            uint8_t expected = a | r;

            check(cpu.reg.A == expected, "OR result");
            check(cpu.reg.F == flags(expected == 0, 0, 0, 0), "OR flags");
        }
    }

    // ==================== XOR ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;

            cpu.XOR(r);

            uint8_t expected = a ^ r;

            check(cpu.reg.A == expected, "XOR result");
            check(cpu.reg.F == flags(expected == 0, 0, 0, 0), "XOR flags");
        }
    }

    // ==================== CP ====================

    for (int a = 0; a < 256; ++a) {
        for (int r = 0; r < 256; ++r) {
            cpu.reg.A = a;

            cpu.CP(r);

            uint8_t expected = (a - r) & 0xFF;

            bool z = expected == 0;
            bool h = (a & 0xF) < (r & 0xF);
            bool c = a < r;

            check(cpu.reg.A == a, "CP preserves A");
            check(cpu.reg.F == flags(z, 1, h, c), "CP flags");
        }
    }

    // ==================== INC8 ====================

    for (int r = 0; r < 256; ++r) {
        for (int old_c = 0; old_c <= 1; ++old_c) {
            cpu.reg.F = old_c << 4;

            uint8_t result = cpu.INC8(r);

            uint8_t expected = r + 1;

            bool z = expected == 0;
            bool h = (r & 0xF) == 0xF;

            check(result == expected, "INC8 result");
            check(cpu.reg.F == flags(z, 0, h, old_c), "INC8 flags");
        }
    }

    // ==================== DEC8 ====================

    for (int r = 0; r < 256; ++r) {
        for (int old_c = 0; old_c <= 1; ++old_c) {
            cpu.reg.F = old_c << 4;

            uint8_t result = cpu.DEC8(r);

            uint8_t expected = r - 1;

            bool z = expected == 0;
            bool h = (r & 0xF) == 0;

            check(result == expected, "DEC8 result");
            check(cpu.reg.F == flags(z, 1, h, old_c), "DEC8 flags");
        }
    }

    // ==================== INC16 ====================

    for (int r = 0; r < 65536; ++r) {
        cpu.reg.F = flags(1, 1, 1, 1);

        uint16_t result = cpu.INC16(r);
        uint16_t expected = r + 1;

        check(result == expected, "INC16 result");
        check(cpu.reg.F == flags(1, 0, 1, 1), "INC16 flags");
    }

    // ==================== DEC16 ====================

    for (int r = 0; r < 65536; ++r) {
        cpu.reg.F = flags(1, 0, 1, 1);

        uint16_t result = cpu.DEC16(r);
        uint16_t expected = r - 1;

        check(result == expected, "DEC16 result");
        check(cpu.reg.F == flags(1, 1, 1, 1), "DEC16 flags");
    }

    // ==================== RESULT ====================

    cout << '\n';
    cout << "ALU TESTS: " << tests << '\n';
    cout << "FAILED:    " << failed << '\n';

    if (failed == 0)
        cout << "ALL ALU TESTS PASSED :3\n";
    else
        cout << "ALU HAS BUGS 💀\n";

    return failed != 0;
}