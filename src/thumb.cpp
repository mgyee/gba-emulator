#include "bus.h"
#include "cpu.h"

void CPU::thumb_msr(uint16_t instr) {
  uint8_t opcode = (instr >> 11) & 0x3;
  uint8_t offset = (instr >> 6) & 0x1f;
  uint8_t rs = (instr >> 3) & 0x7;
  uint8_t rd = instr & 0x7;
  bool carry = get_cc(FLAG::C);
  uint32_t val = get_reg(rs);

  switch (opcode) {
  case 0:
    if (offset != 0) {
      val <<= offset - 1;
      carry = val >> 31;
      val <<= 1;
    }
    break;
  case 1:
    if (offset == 0) {
      offset = 31;
    }
    val >>= offset - 1;
    carry = val & 0x1;
    val >>= 1;
    break;
  case 2:
    if (offset == 0) {
      offset = 31;
    }
    val >>= offset - 1;
    carry = val & 0x1;
    val >>= 1;
    if (val & (1 << (31 - offset))) {
      val |= 0xffffffff << (32 - offset);
    }
    break;
  case 3:
  default:
    break;
  }
  set_reg(rd, val);
  set_cc(FLAG::N, val >> 31);
  set_cc(FLAG::Z, val == 0);
  set_cc(FLAG::C, carry);
};
void CPU::thumb_as(uint16_t instr) {
  uint8_t opcode = (instr >> 9) & 0x3;
  uint8_t rn = (instr >> 6) & 0x7;
  uint8_t rs = (instr >> 3) & 0x7;
  uint8_t rd = instr & 0x7;
  uint32_t op1 = get_reg(rs);
  uint32_t op2 = get_reg(rn);
  uint32_t res;

  switch (opcode) {
  case 0:
    // ADD reg
    res = op1 + op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, (op1 >> 31) + (op2 >> 31) > (res >> 31));
    set_cc(FLAG::V, ((op1 >> 31) == (op2 >> 31)) && (op1 >> 31 != (res >> 31)));
    break;
  case 1:
    // SUB reg
    res = op1 - op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= op2);
    set_cc(FLAG::V,
           ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  case 2:
    // ADD imm
    res = op1 + rn;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, (op1 >> 31) + (rn >> 31) > (res >> 31));
    set_cc(FLAG::V, ((op1 >> 31) == (rn >> 31)) && (op1 >> 31 != (res >> 31)));
    break;
  case 3:
    // SUB imm
    res = op1 - rn;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= rn);
    set_cc(FLAG::V,
           ((op1 >> 31) != (rn >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  default:
    break;
  }
};
void CPU::thumb_mcasi(uint16_t instr) {
  uint8_t opcode = (instr >> 11) & 0x3;
  uint8_t rd = (instr >> 8) & 0x7;
  uint32_t op1 = get_reg(rd);
  uint32_t op2 = (instr) & 0xff;
  uint32_t res;

  switch (opcode) {
  case 0:
    // MOV
    res = op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    break;
  case 1:
    // CMP
    res = op1 - op2;
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= op2);
    set_cc(FLAG::V,
           ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  case 2:
    // ADD
    res = op1 + op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, (op1 >> 31) + (op2 >> 31) > (res >> 31));
    set_cc(FLAG::V, ((op1 >> 31) == (op2 >> 31)) && (op1 >> 31 != (res >> 31)));
    break;
  case 3:
    // SUB
    res = op1 - op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= op2);
    set_cc(FLAG::V,
           ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  }
};
void CPU::thumb_alu(uint16_t instr) {
  uint8_t opcode = (instr >> 6) & 0xf;
  uint8_t rs = (instr >> 3) & 0x7;
  uint8_t rd = instr & 0x7;

  uint32_t op1 = get_reg(rd);
  uint32_t op2 = get_reg(rs);
  uint32_t res;

  switch (opcode) {
  case 0xA:
    // CMP
    res = op1 - op2;
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= op2);
    set_cc(FLAG::V,
           ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  case 0xE:
    // BIC
    res = op1 & ~op2;
    set_reg(rd, res);
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    break;
  default:
    NYI("alu");
    break;
  }
};
void CPU::thumb_hrobx(uint16_t instr) {
  uint8_t opcode = (instr >> 8) & 0x3;
  bool msb_rd = (instr >> 7) & 0x1;
  bool msb_rs = (instr >> 6) & 0x1;
  uint8_t rs = (instr >> 3) & 0x7;
  uint8_t rd = instr & 0x7;
  rs = (msb_rs << 3) | rs;
  rd = (msb_rd << 3) | rd;

  uint32_t op1 = get_reg(rs);
  uint32_t op2 = get_reg(rd);
  uint32_t res;

  switch (opcode) {
  case 0x0:
    res = op1 + op2;
    set_reg(rd, res);
    if (rd == 15) {
      thumb_fetch();
    }
    break;
  case 0x1:
    res = op1 - op2;
    set_cc(FLAG::N, res >> 31);
    set_cc(FLAG::Z, res == 0);
    set_cc(FLAG::C, op1 >= op2);
    set_cc(FLAG::V,
           ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    break;
  case 0x2:
    set_reg(rd, op1);
    if (rd == 15) {
      thumb_fetch();
    }
    break;
  case 0x3:
    set_reg(15, op1 & ~0x1);

    if (op1 & 0x1) {
      uint32_t cpsr = get_cpsr();
      cpsr |= CPU::CONTROL::T;
      set_cpsr(cpsr);
      thumb_fetch();
    } else {
      uint32_t cpsr = get_cpsr();
      cpsr &= ~CPU::CONTROL::T;
      set_cpsr(cpsr);
      arm_fetch();
    }
    break;
  default:
    break;
  }
};
void CPU::thumb_pcrl(uint16_t instr) {
  uint8_t rd = (instr >> 8) & 0x7;
  uint8_t word = instr & 0xff;
  cycle(1);
  set_reg(rd, bus->read32(((get_reg(15) & ~0x2) + (word << 2)),
                          CYCLE_TYPE::NON_SEQ));
};
void CPU::thumb_lsro(uint16_t instr) { NYI("lsro"); };
void CPU::thumb_lssebh(uint16_t instr) { NYI("lssebh"); };
void CPU::thumb_lsio(uint16_t instr) {
  uint8_t opcode = (instr >> 11) & 0x3;
  uint8_t offset = (instr >> 6) & 0x1f;
  uint8_t rb = (instr >> 3) & 0x7;
  uint8_t rd = instr & 0x7;

  switch (opcode) {
  case 0:
    // STR 32-bit
    bus->write32(get_reg(rb) + (offset << 2), get_reg(rd), CYCLE_TYPE::NON_SEQ);
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
    break;
  case 1:
    // LDR 32-bit
    set_reg(rd, bus->read32(get_reg(rb) + (offset << 2), CYCLE_TYPE::NON_SEQ));
    cycle(1);
    break;
  case 2:
    // STRB 8-bit
    bus->write8(get_reg(rb) + offset, get_reg(rd), CYCLE_TYPE::NON_SEQ);
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
    break;
  case 3:
    // LDRB 8-bit
    set_reg(rd, bus->read8(get_reg(rb) + offset, CYCLE_TYPE::NON_SEQ));
    cycle(1);
    break;
  default:
    break;
  }
};
void CPU::thumb_lsh(uint16_t instr) { NYI("lsh"); };
void CPU::thumb_sprls(uint16_t instr) { NYI("sprls"); };
void CPU::thumb_la(uint16_t instr) {
  bool opcode = (instr >> 11) & 0x1;
  uint8_t rd = (instr >> 8) & 0x7;
  uint8_t offset = instr & 0xff;

  if (opcode) {
    set_reg(rd, get_reg(13) + (offset << 2));
  } else {
    set_reg(rd, (get_reg(15) & ~2) + (offset << 2));
  }
};
void CPU::thumb_aosp(uint16_t instr) { NYI("aosp"); };
void CPU::thumb_ppr(uint16_t instr) {
  bool opcode = (instr >> 11) & 0x1;
  bool pc_lr = (instr >> 8) & 0x1;
  uint8_t reg_list = (instr & 0xff);

  bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
  if (opcode) {
    for (int i = 0; i < 8; i++) {
      if ((reg_list >> i) & 0x1) {
        set_reg(i, bus->read32(get_reg(13), CYCLE_TYPE::NON_SEQ));
        set_reg(13, get_reg(13) + 4);
      }
    }
    if (pc_lr) {
      set_reg(15, bus->read32(get_reg(13), CYCLE_TYPE::NON_SEQ) & ~0x1);
      set_reg(13, get_reg(13) + 4);
      thumb_fetch();
    }
    cycle(1);
  } else {
    if (pc_lr) {
      set_reg(13, get_reg(13) - 4);
      bus->write32(get_reg(13), get_reg(14), CYCLE_TYPE::NON_SEQ);
    }
    for (int i = 7; i >= 0; i--) {
      if ((reg_list >> i) & 0x1) {
        set_reg(13, get_reg(13) - 4);
        bus->write32(get_reg(13), get_reg(i), CYCLE_TYPE::NON_SEQ);
      }
    }
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
  }
};
void CPU::thumb_mls(uint16_t instr) {
  bool l = (instr >> 11) & 0x1;
  uint8_t rb = (instr >> 8) & 0x7;
  uint8_t reg_list = (instr & 0xff);

  uint32_t addr = get_reg(rb);

  uint8_t count = 0;

  // not used in ARMv7
  if (reg_list == 0) {
    if (l) {
      set_reg(15, bus->read32(addr, CYCLE_TYPE::NON_SEQ));
      set_reg(rb, addr + 16 * 4);
      thumb_fetch();
    } else {
      bus->write32(rb, get_reg(15), CYCLE_TYPE::NON_SEQ);
      set_reg(rb, addr + 16 * 4);
    }
    return;
  }

  for (int i = 0; i < 8; i++) {
    if ((reg_list >> i) & 0x1) {
      count += 4;
    }
  }

  bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
  set_reg(rb, addr + count);
  cycle(1);
  if (l) {
    for (int i = 0; i < 8; i++) {
      if ((reg_list >> i) & 0x1) {
        set_reg(i, bus->read32(addr, CYCLE_TYPE::SEQ));
        addr += 4;
      }
    }
  } else {
    for (int i = 0; i < 8; i++) {
      if ((reg_list >> i) & 0x1) {
        bus->write32(addr, get_reg(i), CYCLE_TYPE::SEQ);
        addr += 4;
      }
    }
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
  }
};

void CPU::thumb_cb(uint16_t instr) {
  COND cond = static_cast<COND>((instr >> 8) & 0xf);
  uint32_t offset = instr & 0xff;
  if (eval_cond(cond)) {
    if (offset & 0x80) {
      offset |= 0xffffff00;
    }
    offset <<= 1;
    set_reg(15, get_reg(15) + offset);
    thumb_fetch();
  }
};
void CPU::thumb_swi(uint16_t instr) {
  regs_svc[1] = get_reg(15) - 2;
  spsr_svc = cpsr;
  cpsr = (cpsr & 0xffffff00) + 0x93;
  set_reg(15, 0x00000008);
  arm_fetch();
};
void CPU::thumb_ub(uint16_t instr) { NYI("ub"); };
void CPU::thumb_lbl(uint16_t instr) {
  bool h = (instr >> 11) & 0x1;
  uint32_t offset = instr & 0x7ff;
  if (!h) {
    if ((offset >> 10) == 0x1) {
      offset |= 0xFFFFF800;
    }
    set_reg(14, get_reg(15) + (offset << 12));
  } else {
    uint32_t lr = get_reg(14) + (offset << 1);
    set_reg(14, (get_reg(15) - 2) | 0x1);
    set_reg(15, lr);
    thumb_fetch();
  }
};
