#pragma once
#include "cpu.h"

class Bus {
public:
  CPU *cpu;

  void write32(uint32_t addr, uint32_t data, CPU::CYCLE_TYPE type);
  uint32_t read32(uint32_t addr, CPU::CYCLE_TYPE type);
};
