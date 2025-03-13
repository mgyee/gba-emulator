#include "arm.h"
#include "cpu.h"
#include <cstdint>

ARM::ARM(CPU &cpu) : cpu(cpu) {}

bool ARM::is_bx(uint32_t instr) {
  uint32_t bx_format = 0b00000001001011111111111100010000;
  uint32_t mask = 0b00001111111111111111111111110000;
  uint32_t format = instr & mask;
  return format == bx_format;
}

bool ARM::is_bdt(uint32_t instr) {
  uint32_t bdt_format = 0b00001000000000000000000000000000;
  uint32_t mask = 0b00001110000000000000000000000000;
  uint32_t format = instr & mask;
  return format == bdt_format;
}

bool ARM::is_bl(uint32_t instr) {
  uint32_t b_format = 0b00001010000000000000000000000000;
  uint32_t bl_format = 0b00001011000000000000000000000000;
  uint32_t mask = 0b00001111000000000000000000000000;
  uint32_t format = instr & mask;
  return format == b_format || format == bl_format;
}

bool ARM::is_swi(uint32_t instr) {
  uint32_t swi_format = 0b00001111000000000000000000000000;
  uint32_t mask = 0b00001111000000000000000000000000;
  uint32_t format = instr & mask;
  return format == swi_format;
}

bool ARM::is_und(uint32_t instr) {
  uint32_t und_format = 0b00000110000000000000000000010000;
  uint32_t mask = 0b00001110000000000000000000010000;
  uint32_t format = instr & mask;
  return format == und_format;
}

bool ARM::is_sdt(uint32_t instr) {
  uint32_t sdt_format = 0b00000100000000000000000000000000;
  uint32_t mask = 0b00001100000000000000000000000000;
  uint32_t format = instr & mask;
  return format == sdt_format;
}

bool ARM::is_sds(uint32_t instr) {
  uint32_t sds_format = 0b00000001000000000000000010010000;
  uint32_t mask = 0b00001111100000000000111111110000;
  uint32_t format = instr & mask;
  return format == sds_format;
}

bool ARM::is_mul(uint32_t instr) {
  uint32_t mul_format = 0b00000000000000000000000010010000;
  uint32_t mull_format = 0b00000000100000000000000010010000;
  uint32_t mask = 0b00001111100000000000000011110000;
  uint32_t format = instr & mask;
  return format == mul_format || format == mull_format;
}

bool ARM::is_hdtr(uint32_t instr) {
  uint32_t hdtr_format = 0b00000000000000000000000010010000;
  uint32_t mask = 0b00001110010000000000111110010000;
  uint32_t format = instr & mask;
  return format == hdtr_format;
}

bool ARM::is_hdti(uint32_t instr) {
  uint32_t hdti_format = 0b00000000010000000000000010010000;
  uint32_t mask = 0b00001110010000000000000010010000;
  uint32_t format = instr & mask;
  return format == hdti_format;
}

bool ARM::is_pmrs(uint32_t instr) {
  uint32_t pmrs_format = 0b00000001000011110000000000000000;
  uint32_t mask = 0b00001111101111110000000000000000;
  uint32_t format = instr & mask;
  return format == pmrs_format;
}

bool ARM::is_pmsr(uint32_t instr) {
  uint32_t pmrs_format = 0b00000001001000001111000000000000;
  uint32_t mask = 0b00001101101100001111000000000000;
  uint32_t format = instr & mask;
  return format == pmrs_format;
}

bool ARM::is_dproc(uint32_t instr) {
  uint32_t dproc_format = 0b00000000000000000000000000000000;
  uint32_t mask = 0b00001100000000000000000000000000;
  uint32_t format = instr & mask;
  return format == dproc_format;
}

void ARM::bx(uint32_t instr) {
  uint8_t reg = instr & 0xf;
  uint32_t reg_val = cpu.get_reg(reg);
  if (reg_val & 0x1) {
    uint32_t cpsr = cpu.get_cpsr();
    cpsr |= CPU::CONTROL::T;
    cpu.set_cpsr(cpsr);
    cpu.set_reg(15, reg_val & ~0x1);
  } else {
    uint32_t cpsr = cpu.get_cpsr();
    cpsr &= ~CPU::CONTROL::T;
    cpu.set_cpsr(cpsr);
    cpu.set_reg(15, reg_val & ~0x3);
  }
}

void ARM::run() {
  uint32_t instr = cpu.arm_fetch_next();

  if (cpu.eval_cond(instr)) {
    // if (is_bx(instr)) {
    //   bx(instr);
    // } else if (is_bdt(instr)) {
    //   bdt(instr);
    // } else if (is_bl(instr)) {
    //   bl(instr);
    // } else if (is_swi(instr)) {
    //   swi(instr);
    // } else if (is_und(instr)) {
    //   und(instr);
    // } else if (is_sdt(instr)) {
    //   sdt(instr);
    // } else if (is_sds(instr)) {
    //   sds(instr);
    // } else if (is_mul(instr)) {
    //   mul(instr);
    // } else if (is_hdtr(instr)) {
    //   hdtr(instr);
    // } else if (is_hdti(instr)) {
    //   hdti(instr);
    // } else if (is_pmrs(instr)) {
    //   pmrs(instr);
    // } else if (is_pmsr(instr)) {
    //   pmsr(instr);
    // } else if (is_dproc(instr)) {
    //   dproc(instr);
    // }
  }
}
