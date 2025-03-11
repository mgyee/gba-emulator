#include "cpu.hpp"

bool CPU::eval_cond(uint32_t instr) {
  uint32_t cpsr = this->cpsr;
  uint8_t cond = (instr >> 28) & 0xf;
  switch (cond) {
  case EQ:
    return cpsr & Z;
  case NE:
    return !(cpsr & Z);
  case CS:
    return cpsr & C;
  case CC:
    return !(cpsr & C);
  case MI:
    return cpsr & N;
  case PL:
    return !(cpsr & N);
  case VS:
    return cpsr & V;
  case VC:
    return !(cpsr & V);
  case HI:
    return (cpsr & C) & !(cpsr & Z);
  case LS:
    return !(cpsr & C) | (cpsr & Z);
  case GE:
    return (cpsr & N) == (cpsr & V);
  case LT:
    return (cpsr & N) ^ (cpsr & V);
  case GT:
    return !(cpsr & Z) & ((cpsr & N) == (cpsr & V));
  case LE:
    return (cpsr & Z) | ((cpsr & N) ^ (cpsr & V));
  case AL:
    return true;
  default:
    return false;
  }
}
