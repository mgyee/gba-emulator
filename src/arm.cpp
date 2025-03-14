#include "cpu.h"
#include <cstdint>

bool CPU::is_bx(uint32_t instr) {
  uint32_t bx_format = 0b00000001001011111111111100010000;
  uint32_t mask = 0b00001111111111111111111111110000;
  uint32_t format = instr & mask;
  return format == bx_format;
}

bool CPU::is_bdt(uint32_t instr) {
  uint32_t bdt_format = 0b00001000000000000000000000000000;
  uint32_t mask = 0b00001110000000000000000000000000;
  uint32_t format = instr & mask;
  return format == bdt_format;
}

bool CPU::is_bl(uint32_t instr) {
  uint32_t b_format = 0b00001010000000000000000000000000;
  uint32_t bl_format = 0b00001011000000000000000000000000;
  uint32_t mask = 0b00001111000000000000000000000000;
  uint32_t format = instr & mask;
  return format == b_format || format == bl_format;
}

bool CPU::is_swi(uint32_t instr) {
  uint32_t swi_format = 0b00001111000000000000000000000000;
  uint32_t mask = 0b00001111000000000000000000000000;
  uint32_t format = instr & mask;
  return format == swi_format;
}

bool CPU::is_und(uint32_t instr) {
  uint32_t und_format = 0b00000110000000000000000000010000;
  uint32_t mask = 0b00001110000000000000000000010000;
  uint32_t format = instr & mask;
  return format == und_format;
}

bool CPU::is_sdt(uint32_t instr) {
  uint32_t sdt_format = 0b00000100000000000000000000000000;
  uint32_t mask = 0b00001100000000000000000000000000;
  uint32_t format = instr & mask;
  return format == sdt_format;
}

bool CPU::is_sds(uint32_t instr) {
  uint32_t sds_format = 0b00000001000000000000000010010000;
  uint32_t mask = 0b00001111100000000000111111110000;
  uint32_t format = instr & mask;
  return format == sds_format;
}

bool CPU::is_mul(uint32_t instr) {
  uint32_t mul_format = 0b00000000000000000000000010010000;
  uint32_t mull_format = 0b00000000100000000000000010010000;
  uint32_t mask = 0b00001111100000000000000011110000;
  uint32_t format = instr & mask;
  return format == mul_format || format == mull_format;
}

bool CPU::is_hdtr(uint32_t instr) {
  uint32_t hdtr_format = 0b00000000000000000000000010010000;
  uint32_t mask = 0b00001110010000000000111110010000;
  uint32_t format = instr & mask;
  return format == hdtr_format;
}

bool CPU::is_hdti(uint32_t instr) {
  uint32_t hdti_format = 0b00000000010000000000000010010000;
  uint32_t mask = 0b00001110010000000000000010010000;
  uint32_t format = instr & mask;
  return format == hdti_format;
}

bool CPU::is_psrtmrs(uint32_t instr) {
  uint32_t pmrs_format = 0b00000001000011110000000000000000;
  uint32_t mask = 0b00001111101111110000000000000000;
  uint32_t format = instr & mask;
  return format == pmrs_format;
}

bool CPU::is_psrtmsr(uint32_t instr) {
  uint32_t pmrs_format = 0b00000001001000001111000000000000;
  uint32_t mask = 0b00001101101100001111000000000000;
  uint32_t format = instr & mask;
  return format == pmrs_format;
}

bool CPU::is_dproc(uint32_t instr) {
  uint32_t dproc_format = 0b00000000000000000000000000000000;
  uint32_t mask = 0b00001100000000000000000000000000;
  uint32_t format = instr & mask;
  return format == dproc_format;
}

void CPU::bx(uint32_t instr) {
  uint8_t rn = instr & 0xf;
  uint32_t reg_val = get_reg(rn);
  if (reg_val & 0x1) {
    uint32_t cpsr = get_cpsr();
    cpsr |= CPU::CONTROL::T;
    set_cpsr(cpsr);
    set_reg(15, reg_val & ~0x1);

    thumb_fetch();
  } else {
    uint32_t cpsr = get_cpsr();
    cpsr &= ~CPU::CONTROL::T;
    set_cpsr(cpsr);
    set_reg(15, reg_val & ~0x3);

    arm_fetch(); // 1N + 1S, next fetch -> +1S
  }
}

void CPU::bdt(uint32_t instr) {
  bool p = (instr >> 24) & 0x1; // pre/post
  bool u = (instr >> 23) & 0x1; // up/down
  bool s = (instr >> 22) & 0x1; // psr & force user mode
  bool w = (instr >> 21) & 0x1; // writeback
  bool l = (instr >> 20) & 0x1; // load/store

  uint8_t rn = (instr >> 16) & 0xf;
  uint16_t reg_list = instr & 0xffff;

  uint32_t base = get_reg(rn);
  uint32_t offset = u ? 4 : -4;

  uint32_t bank = 0;

  bool r15_transfer = (reg_list >> 0xf) & 0x1;

  if (s) {
    if (l && r15_transfer) {
      cpsr = get_psr();
    } else {
      bank = cpsr;
      cpsr = (cpsr & ~0xff) | MODE::USR;
    }
  }

  if (reg_list == 0) {
    reg_list |= (1 << 0xf);
    set_reg(rn, base + (16 * offset));
    r15_transfer = true;
  } else {
  }
}

void CPU::bl(uint32_t instr) {
  bool l = (instr >> 24) & 0x1; // link
  uint32_t offset = instr & 0xffffff;
  if (offset & 0x800000) {
    offset |= 0xff000000;
  }
  offset <<= 2;

  if ((cpsr & CONTROL::T) == 0x1) {
    offset >>= 1;
  }

  if (l) {
    set_reg(14, get_reg(15) - 4);
  }

  uint32_t pc = get_reg(15);
  set_reg(15, pc + offset);

  arm_fetch(); // 1N + 1S, next fetch -> +1S
}
