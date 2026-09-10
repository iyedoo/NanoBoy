#include "mem.h"

uint8_t RAM::read(uint16_t address) { return mem[address]; }
void RAM::write(uint16_t address, uint8_t val) { mem[address] = val; }