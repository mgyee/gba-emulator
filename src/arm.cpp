#include "bus.h"
#include "cpu.h"
#include <cstdint>
#include <iostream>

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

bool CPU::is_hdtri(uint32_t instr) {
  uint32_t hdtr_format = 0b00000000000000000000000010010000;
  uint32_t hdtr_mask = 0b00001110010000000000111110010000;
  uint32_t format1 = instr & hdtr_mask;
  uint32_t hdti_format = 0b00000000010000000000000010010000;
  uint32_t hdti_mask = 0b00001110010000000000000010010000;
  uint32_t format2 = instr & hdti_mask;
  return format1 == hdtr_format || format2 == hdti_format;
}

// bool CPU::is_hdti(uint32_t instr) {
//   uint32_t hdti_format = 0b00000000010000000000000010010000;
//   uint32_t mask = 0b00001110010000000000000010010000;
//   uint32_t format = instr & mask;
//   return format == hdti_format;
// }

bool CPU::is_psrt(uint32_t instr) {
  uint32_t mrs_format = 0b00000001000011110000000000000000;
  uint32_t mrs_mask = 0b00001111101111110000000000000000;
  uint32_t msr_format = 0b00000001001000001111000000000000;
  uint32_t msr_mask = 0b00001101101100001111000000000000;
  uint32_t format1 = instr & mrs_mask;
  uint32_t format2 = instr & msr_mask;
  return format1 == mrs_format || format2 == msr_format;
}

// bool CPU::is_psrtmsr(uint32_t instr) {
//   uint32_t pmrs_format = 0b00000001001000001111000000000000;
//   uint32_t mask = 0b00001101101100001111000000000000;
//   uint32_t format = instr & mask;
//   return format == pmrs_format;
// }

bool CPU::is_dproc(uint32_t instr) {
  uint32_t dproc_format = 0b00000000000000000000000000000000;
  uint32_t mask = 0b00001100000000000000000000000000;
  uint32_t format = instr & mask;
  return format == dproc_format;
}

bool CPU::barrel_shift(uint32_t &val, SHIFT shift_type, uint8_t shift_amount,
                       bool shift_by_reg) {
  bool carry_out = false;
  switch (shift_type) {
  case LSL:
    if (shift_amount == 0) {
      carry_out = (cpsr & C) == C;
    } else if (shift_amount == 32) {
      carry_out = val & 0x1;
      val = 0;
    } else {
      carry_out = shift_amount > 32 ? 0 : (val << (shift_amount - 1)) >> 31;
      val = shift_amount > 32 ? 0 : val << shift_amount;
    }
    break;
  case LSR:
    if ((!shift_by_reg && shift_amount == 0) || shift_amount == 32) {
      carry_out = val >> 31;
      val = 0;
    } else {
      carry_out = shift_amount > 32 ? 0 : (val >> (shift_amount - 1)) & 0x1;
      val = shift_amount > 32 ? 0 : val >> shift_amount;
    }
    break;
  case ASR:
    if (!shift_by_reg && shift_amount == 0) {
      carry_out = val >> 31;
      val = carry_out ? 0xffffffff : 0;
    } else {
      carry_out = shift_amount > 31
                      ? val >> 31
                      : (static_cast<int32_t>(val) >> (shift_amount - 1)) & 0x1;
      val = shift_amount > 31 ? val >> 31 ? 0xffffffff : 0
                              : static_cast<int32_t>(val) >> shift_amount;
    }
    break;
  case ROR:
    if (!shift_by_reg && shift_amount == 0) {
      bool carry_in = (cpsr & C) == C;
      carry_out = val & 0x1;
      val = val >> 1 | carry_in << 31;
    } else {
      carry_out = val & 0x1;
      shift_amount %= 32;
      val = (val >> shift_amount) | (val << (32 - shift_amount));
    }
    break;
  default:
    break;
  }
  return carry_out;
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

  uint32_t addr = get_reg(rn);

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

  uint8_t first_reg = 0;
  uint8_t size = 0;

  if (reg_list != 0) {
    for (int i = 15; i >= 0; i--) {
      if (~reg_list & (1 << i)) {
        continue;
      }
      first_reg = i;
      size += 4;
    }
  } else {
    reg_list = (1 << 0xf);
    r15_transfer = true;
    size = 64;
  }

  uint32_t addr_copy = addr;

  if (!u) {
    p = !p;
    addr -= size;
    addr_copy -= size;
  } else {
    addr_copy += size;
  }

  bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);

  for (int i = first_reg; i < 16; i++) {
    if (!(reg_list & (1 << i))) {
      continue;
    }
    if (p)
      addr += 4;
    if (l) {
      uint32_t val = bus->read32(addr, CYCLE_TYPE::SEQ);
      if (w && i == first_reg) {
        set_reg(rn, addr_copy);
      }
      set_reg(i, val);
    } else {
      bus->write32(addr, get_reg(i) + ((i == 15) << 2), CYCLE_TYPE::SEQ);
      if (w && i == first_reg) {
        set_reg(rn, addr_copy);
      }
    }

    if (!p)
      addr -= 4;
  }

  if (bank) {
    cpsr = bank;
  }

  if (l) {
    // cylce 1I
    if (r15_transfer) {
      arm_fetch(); // 1N + 1S, next fetch -> +1S
    }
  } else {
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ);
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

void CPU::swi(uint32_t instr) {}

void CPU::und(uint32_t instr) {}

void CPU::sdt(uint32_t instr) {
  bool i = (instr >> 25) & 0x1; // reg/imm
  bool p = (instr >> 24) & 0x1; // pre/post
  bool u = (instr >> 23) & 0x1; // up/down
  bool b = (instr >> 22) & 0x1; // byte/word
  bool t = (instr >> 21) & 0x1; // force non-priv
  bool w = (instr >> 21) & 0x1; // writeback
  bool l = (instr >> 20) & 0x1; // load/store
  uint8_t rn = (instr >> 16) & 0xf;
  uint8_t rd = (instr >> 12) & 0xf;

  uint32_t offset;

  if (i) {
    uint8_t shift_amount = (instr >> 7) & 0x1f;
    SHIFT shift_type = static_cast<SHIFT>((instr >> 5) & 0x3);
    uint8_t rm = instr & 0xf;

    offset = get_reg(rm);
    barrel_shift(offset, shift_type, shift_amount, false);
  } else {
    offset = instr & 0xfff;
  }

  if (!u) {
    offset = -offset;
  }

  uint32_t addr = get_reg(rn) + (p ? offset : 0);

  if (l) {
    // read does 1N
    uint32_t val;
    if (b) {
      val = bus->read8(addr, CYCLE_TYPE::NON_SEQ);
    } else {
      // barrel_shift(addr, SHIFT::ROR, (addr & 0x3) * 8, true);
      val = bus->read32(addr, CYCLE_TYPE::NON_SEQ);
    }
    set_reg(rd, val);
  } else {
    // write does 1N
    uint32_t val = get_reg(rd) + ((rd == 15) << 2);
    if (rd == 15) {
      val += 4;
    }
    if (b) {
      bus->write8(addr, val, CYCLE_TYPE::NON_SEQ);
    } else {
      bus->write32(addr, val, CYCLE_TYPE::NON_SEQ);
    }
    bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ); // next fetch is 1N
  }

  if (!p || (p && w)) {
    if (!l || !(rn == rd)) {
      set_reg(rn, get_reg(rn) + ((rn == 15) << 2) + offset);
    }
  }

  cycle(1);

  if (l && (rd == 15)) {
    arm_fetch(); // 1N + 1S
  }
}

void CPU::sds(uint32_t instr) {}

void CPU::mul(uint32_t instr) {}

void CPU::hdtri(uint32_t instr) {
  bool p = (instr >> 24) & 0x1; // pre/post
  bool u = (instr >> 23) & 0x1; // up/down
  bool i = (instr >> 22) & 0x1; // imm/reg
  bool w = (instr >> 21) & 0x1; // writeback
  bool l = (instr >> 20) & 0x1; // load/store
  uint8_t rn = (instr >> 16) & 0xf;
  uint8_t rd = (instr >> 12) & 0xf;

  uint8_t opcode = (instr >> 5) & 0x3;
  uint32_t offset;

  if (i) {
    uint8_t offset_hi = (instr >> 8) & 0xf;
    uint8_t offset_lo = instr & 0xf;
    offset = (offset_hi << 4) | offset_lo;
  } else {
    uint8_t rm = instr & 0xf;
    offset = get_reg(rm);
    // barrel_shift(offset, SHIFT::ROR, (offset & 0x3) * 8, true);
  }

  if (!u) {
    offset = -offset;
  }

  uint32_t addr = get_reg(rn) + (p ? offset : 0);

  uint32_t val;
  switch (opcode) {
  case 0x1:
    if (l) {
      if (addr & 1) {
        val = bus->read16(addr, CYCLE_TYPE::NON_SEQ);
        barrel_shift(val, SHIFT::ROR, 8, true);
      } else {
        val = bus->read16(addr, CYCLE_TYPE::NON_SEQ);
      }
      set_reg(rd, val);
    } else {
      bus->write16(addr, get_reg(rd), CYCLE_TYPE::NON_SEQ);
      bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ); // next fetch is 1N
    }
    break;
  case 0x2:
    if (l) {
      val = bus->read8(addr, CYCLE_TYPE::NON_SEQ);
      if (val & 0x80) {
        val |= 0xffffff00;
      }
      set_reg(rd, val);
    } else {
      bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ); // next fetch is 1N
    }
    break;
  case 0x3:
    if (l) {
      if (addr & 1) {
        val = bus->read8(addr, CYCLE_TYPE::NON_SEQ);
        if (val & 0x80) {
          val |= 0xffffff00;
        }
      } else {
        val = bus->read16(addr, CYCLE_TYPE::NON_SEQ);
        if (val & 0x8000) {
          val |= 0xffff0000;
        }
      }
      set_reg(rd, val);
    } else {
      bus->set_last_cycle_type(CYCLE_TYPE::NON_SEQ); // next fetch is 1N
    }
    break;
  default:
    break;
  }

  if (!p || (p && w)) {
    if (!l || !(rn == rd)) {
      set_reg(rn, get_reg(rn) + ((rn == 15) << 2) + offset);
    }
  }

  cycle(1);

  if (l && (rd == 15)) {
    arm_fetch(); // 1N + 1S
  }
}

void CPU::psrt(uint32_t instr) {
  bool i = (instr >> 25) & 0x1;      // imm/reg
  bool psr = (instr >> 22) & 0x1;    // spsr/cpsr
  bool opcode = (instr >> 21) & 0x1; // msr/mrs

  if (opcode) {
    bool f = (instr >> 19) & 0x1; // flag
    bool s = (instr >> 18) & 0x1; // status
    bool x = (instr >> 17) & 0x1; // extension
    bool c = (instr >> 16) & 0x1; // control
    uint32_t op;
    if (i) {
      uint8_t imm = instr & 0xff;
      uint8_t shift_amount = (instr >> 8) & 0xf;
      op = imm;
      barrel_shift(op, SHIFT::ROR, shift_amount * 2, true);
    } else {
      uint8_t rm = instr & 0xf;
      op = get_reg(rm);
    }

    uint32_t mask = 0;
    if (f) {
      mask |= 0xff000000;
    }
    if (s) {
      mask |= 0x00ff0000;
    }
    if (x) {
      mask |= 0x0000ff00;
    }
    if (c) {
      mask |= 0x000000ff;
    }

    if (psr) {
      uint32_t psr = get_psr();
      psr &= ~mask;
      psr |= op & mask;
      set_psr(psr);
    } else {
      cpsr &= ~mask;
      cpsr |= op & mask;
    }
  } else {
    uint8_t rd = (instr >> 12) & 0xf;
    if (psr) {
      set_reg(rd, get_psr());
    } else {
      set_reg(rd, cpsr);
    }
  }
}

void CPU::dproc(uint32_t instr) {
  bool i = (instr >> 25) & 0x1; // imm/reg
  uint8_t opcode = (instr >> 21) & 0xf;
  bool s = (instr >> 20) & 0x1; // set
  uint8_t rn = (instr >> 16) & 0xf;
  uint8_t rd = (instr >> 12) & 0xf;

  uint32_t op1 = get_reg(rn);
  uint32_t op2;

  bool carry = get_cc(C);

  bool r15_transfer = (rd == 15);

  if (i) {
    uint8_t imm = instr & 0xff;
    uint8_t shift_amount = (instr >> 8) & 0xf;
    op2 = imm;
    carry = barrel_shift(op2, SHIFT::ROR, shift_amount * 2, true);
  } else {
    uint8_t rm = instr & 0xf;
    op2 = get_reg(rm);
    SHIFT shift_type = static_cast<SHIFT>((instr >> 5) & 0x3);
    bool shift_by_reg = (instr >> 4) & 0x1;

    if (shift_by_reg) {
      uint8_t rs = (instr >> 8) & 0xf;
      uint8_t shift_amount = get_reg(rs) & 0xf;
      if (rn == 15) {
        op1 += 4;
      }
      if (rm == 15) {
        op2 += 4;
      }
      carry = barrel_shift(op2, shift_type, shift_amount, true);
    } else {
      uint8_t amount = (instr >> 7) & 0x1f;
      carry = barrel_shift(op2, shift_type, amount, false);
    }

    // clock 1I
  }

  uint32_t res;
  switch (opcode) {
  case DPROC_OPCODE::AND:
    // std::cout << "AND" << std::endl;
    res = op1 & op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::EOR:
    // std::cout << "EOR" << std::endl;
    res = op1 ^ op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::SUB:
    // std::cout << "SUB" << std::endl;
    res = op1 - op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, op1 >= op2);
      set_cc(FLAG::V,
             ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::RSB:
    // std::cout << "RSB" << std::endl;
    res = op2 - op1;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, op2 >= op1);
      set_cc(FLAG::V,
             ((op2 >> 31) != (op1 >> 31)) && ((op2 >> 31) != (res >> 31)));
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::ADD:
    // std::cout << "ADD" << std::endl;
    res = op1 + op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, (op1 >> 31) + (op2 >> 31) > (res >> 31));
      set_cc(FLAG::V,
             ((op1 >> 31) == (op2 >> 31)) && (op1 >> 31 != (res >> 31)));
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::ADC:
    // std::cout << "ADC" << std::endl;
    res = op1 + op2 + get_cc(FLAG::C);
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, (op1 >> 31) + (op2 >> 31) + carry > (res >> 31));
      set_cc(FLAG::V,
             ((op1 >> 31) == (op2 >> 31)) && (op1 >> 31 != (res >> 31)));
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::SBC:
    // std::cout << "SBC" << std::endl;
    res = op1 - op2 - !get_cc(FLAG::C);
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, static_cast<uint64_t>(op1) >=
                          static_cast<uint64_t>(op2) + !get_cc(FLAG::C));
      set_cc(FLAG::V,
             ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::RSC:
    // std::cout << "RSC" << std::endl;
    res = op2 - op1 - !get_cc(FLAG::C);
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, static_cast<uint64_t>(op2) >=
                          static_cast<uint64_t>(op1) + !get_cc(FLAG::C));
      set_cc(FLAG::V,
             ((op2 >> 31) != (op1 >> 31)) && ((op2 >> 31) != (res >> 31)));
    }
    break;
  case DPROC_OPCODE::TST:
    // std::cout << "TST" << std::endl;
    res = op1 & op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    break;
  case DPROC_OPCODE::TEQ:
    // std::cout << "TEQ" << std::endl;
    res = op1 ^ op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    break;
  case DPROC_OPCODE::CMP:
    // std::cout << "CMP" << std::endl;
    res = op1 - op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, op1 >= op2);
      set_cc(FLAG::V,
             ((op1 >> 31) != (op2 >> 31)) && ((op1 >> 31) != (res >> 31)));
    }
    break;
  case DPROC_OPCODE::CMN:
    // std::cout << "CMN" << std::endl;
    res = op1 + op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, (op1 >> 31) + (op2 >> 31) > (res >> 31));
      set_cc(FLAG::V,
             ((op1 >> 31) == (op2 >> 31)) && (op1 >> 31 != (res >> 31)));
    }
    break;
  case DPROC_OPCODE::ORR:
    // std::cout << "ORR" << std::endl;
    res = op1 | op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::MOV:
    // std::cout << "MOV" << std::endl;
    res = op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::BIC:
    // std::cout << "BIC" << std::endl;
    res = op1 & (~op2);
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  case DPROC_OPCODE::MVN:
    // std::cout << "MVN" << std::endl;
    res = ~op2;
    if (s) {
      set_cc(FLAG::N, res >> 31);
      set_cc(FLAG::Z, res == 0);
      set_cc(FLAG::C, carry);
    }
    set_reg(rd, res);
    break;
  default:
    break;
  }

  if (r15_transfer) {
    arm_fetch(); // 1N + 1S
    if (s)
      cpsr = get_psr();
  }
}
