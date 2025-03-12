#include "cpu.h"
#include "bus.h"

CPU::MODE CPU::get_mode() { return static_cast<MODE>(this->cpsr & CONTROL::M); }

uint32_t CPU::get_cpsr() { return cpsr; }
void CPU::set_cpsr(uint32_t val) { cpsr = val; }

void CPU::cycle(uint64_t count) { cycles += count; }

void CPU::arm_fetch() {
  pipeline[0] = bus->read32(regs[15], CYCLE_TYPE::NON_SEQ);
  pipeline[1] = bus->read32(regs[15] + 4, CYCLE_TYPE::SEQ);
}

void CPU::arm_fetch_next() {
  pipeline[1] = bus->read32(regs[15] + 4, CYCLE_TYPE::NON_SEQ);
}

void CPU::thumb_fetch() {
  pipeline[0] = pipeline[1];
  pipeline[1] = 0;
}

void CPU::thumb_fetch_next() {
  pipeline[0] = pipeline[1];
  pipeline[1] = 0;
}

uint32_t CPU::get_reg(uint8_t reg) {
  MODE mode = get_mode();
  switch (reg) {
  case 0x0:
    return regs[0];
  case 0x1:
    return regs[1];
  case 0x2:
    return regs[2];
  case 0x3:
    return regs[3];
  case 0x4:
    return regs[4];
  case 0x5:
    return regs[5];
  case 0x6:
    return regs[6];
  case 0x7:
    return regs[7];
  case 0x8:
    if (mode == FIQ)
      return regs_fiq[0];
    return regs[8];
  case 0x9:
    if (mode == FIQ)
      return regs_fiq[1];
    return regs[9];
  case 0xA:
    if (mode == FIQ)
      return regs_fiq[2];
    return regs[10];
  case 0xB:
    if (mode == FIQ)
      return regs_fiq[3];
    return regs[11];
  case 0xC:
    if (mode == FIQ)
      return regs_fiq[4];
    return regs[12];
  case 0xD:
    switch (mode) {
    case USR:
    case SYS:
      return regs[13];
    case FIQ:
      return regs_fiq[5];
    case IRQ:
      return regs_irq[0];
    case SVC:
      return regs_svc[0];
    case ABT:
      return regs_abt[0];
    case UND:
      return regs_und[0];
    default:
      return 0;
    }
  case 0xE:
    switch (mode) {
    case USR:
    case SYS:
      return regs[14];
    case FIQ:
      return regs_fiq[6];
    case IRQ:
      return regs_irq[1];
    case SVC:
      return regs_svc[1];
    case ABT:
      return regs_abt[1];
    case UND:
      return regs_und[1];
    default:
      return 0;
    }
  case 0xF:
    return regs[15];
  default:
    return 0;
  }
}

void CPU::set_reg(uint8_t reg, uint32_t val) {
  MODE mode = get_mode();
  switch (reg) {
  case 0x0:
    regs[0] = val;
    break;
  case 0x1:
    regs[1] = val;
    break;
  case 0x2:
    regs[2] = val;
    break;
  case 0x3:
    regs[3] = val;
    break;
  case 0x4:
    regs[4] = val;
    break;
  case 0x5:
    regs[5] = val;
    break;
  case 0x6:
    regs[6] = val;
    break;
  case 0x7:
    regs[7] = val;
    break;
  case 0x8:
    if (mode == FIQ) {
      regs_fiq[0] = val;
    } else {
      regs[8] = val;
    }
    break;
  case 0x9:
    if (mode == FIQ) {
      regs_fiq[1] = val;
    } else {
      regs[9] = val;
    }
    break;
  case 0xA:
    if (mode == FIQ) {
      regs_fiq[2] = val;
    } else {
      regs[10] = val;
    }
    break;
  case 0xB:
    if (mode == FIQ) {
      regs_fiq[3] = val;
    } else {
      regs[11] = val;
    }
    break;
  case 0xC:
    if (mode == FIQ) {
      regs_fiq[4] = val;
    } else {
      regs[12] = val;
    }
    break;
  case 0xD:
    switch (mode) {
    case USR:
    case SYS:
      regs[13] = val;
      break;
    case FIQ:
      regs_fiq[5] = val;
      break;
    case IRQ:
      regs_irq[0] = val;
      break;
    case SVC:
      regs_svc[0] = val;
      break;
    case ABT:
      regs_abt[0] = val;
      break;
    case UND:
      regs_und[0] = val;
      break;
    default:
      break;
    }
    break;
  case 0xE:
    switch (mode) {
    case USR:
    case SYS:
      regs[14] = val;
      break;
    case FIQ:
      regs_fiq[6] = val;
      break;
    case IRQ:
      regs_irq[1] = val;
      break;
    case SVC:
      regs_svc[1] = val;
      break;
    case ABT:
      regs_abt[1] = val;
      break;
    case UND:
      regs_und[1] = val;
      break;
    default:
      break;
    }
    break;
  case 0xF:
    if (cpsr & CONTROL::T) {
      regs[15] = val & ~0x1;
      thumb_fetch();
    } else {
      regs[15] = val & ~0x3;
      arm_fetch();
    }
    break;
  default:
    break;
  }
}

uint32_t CPU::get_psr() {
  switch (get_mode()) {
  case USR:
  case SYS:
    return cpsr;
  case FIQ:
    return spsr_fiq;
  case IRQ:
    return spsr_irq;
  case SVC:
    return spsr_svc;
  case ABT:
    return spsr_abt;
  case UND:
    return spsr_und;
  default:
    return 0;
  }
}

void CPU::set_psr(uint32_t val) {
  switch (get_mode()) {
  case USR:
  case SYS:
    cpsr = val;
    break;
  case FIQ:
    spsr_fiq = val;
    break;
  case IRQ:
    spsr_irq = val;
    break;
  case SVC:
    spsr_svc = val;
    break;
  case ABT:
    spsr_abt = val;
    break;
  case UND:
    spsr_und = val;
    break;
  default:
    break;
  }
}

bool CPU::get_cc(FLAG f) { return CPU::get_psr() & f; }
void CPU::set_cc(FLAG f, bool val) {
  uint32_t psr = get_psr();
  if (val) {
    psr |= f;
  } else {
    psr &= ~f;
  }
  set_psr(psr);
}

bool CPU::eval_cond(uint32_t instr) {
  uint8_t cond = (instr >> 28) & 0xf;
  switch (cond) {
  case EQ:
    return get_cc(Z);
  case NE:
    return !get_cc(Z);
  case CS:
    return get_cc(C);
  case CC:
    return !get_cc(C);
  case MI:
    return get_cc(N);
  case PL:
    return !get_cc(N);
  case VS:
    return get_cc(V);
  case VC:
    return !get_cc(V);
  case HI:
    return get_cc(C) & !get_cc(Z);
  case LS:
    return !get_cc(C) | get_cc(Z);
  case GE:
    return (get_cc(N) == get_cc(V));
  case LT:
    return (get_cc(N) != get_cc(V));
  case GT:
    return !get_cc(Z) & (get_cc(N) == get_cc(V));
  case LE:
    return get_cc(Z) | (get_cc(N) != get_cc(V));
  case AL:
    return true;
  default:
    return false;
  }
}
