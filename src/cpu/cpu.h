#pragma once
#include <cstdint>

#include "../memory/mem.h"

class REG {
public:
    uint8_t A, F; 
    uint8_t B, C; 
    uint8_t D, E; 
    uint8_t H, L;

    void flags(bool z, bool n, bool h, bool c);

    uint16_t AF(); void sAF(uint16_t val);
    uint16_t BC(); void sBC(uint16_t val);
    uint16_t DE(); void sDE(uint16_t val);
    uint16_t HL(); void sHL(uint16_t val);
    
    uint16_t PC, SP;
};

class CPU {
public:
    REG reg;
    
    RAM& ram;

    CPU(RAM& ram);

    void ADD(uint8_t r);

    void init();
    void step();
};