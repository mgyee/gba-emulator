#pragma once
#include <cstdint>

class Bus;
class ARM;

class CPU {

public:
  enum CYCLE_TYPE {
    NON_SEQ,
    SEQ,
  };

  // instr[31:28]
  enum COND {
    EQ = 0x0,
    NE = 0x1,
    CS = 0x2,
    CC = 0x3,
    MI = 0x4,
    PL = 0x5,
    VS = 0x6,
    VC = 0x7,
    HI = 0x8,
    LS = 0x9,
    GE = 0xa,
    LT = 0xb,
    GT = 0xc,
    LE = 0xd,
    AL = 0xe,
  };

  // cspr[7:0] = I F T M[4:0]
  enum CONTROL {
    I = 1 << 7,
    F = 1 << 6,
    T = 1 << 5,
    M = 1 << 4 | 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0,
  };

  CPU(Bus &bus);

  bool eval_cond(uint32_t instr);

  uint32_t get_reg(uint8_t reg);
  void set_reg(uint8_t reg, uint32_t val);

  uint32_t get_cpsr();
  void set_cpsr(uint32_t val);

  void cycle(uint64_t count);

  void arm_fetch();
  uint32_t arm_fetch_next();

  void thumb_fetch();
  void thumb_fetch_next();

private:
  // cspr[31:28] = N Z C V
  enum FLAG {
    N = 1 << 31,
    Z = 1 << 30,
    C = 1 << 29,
    V = 1 << 28,
  };

  // cspr[4:0]
  enum MODE {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1b,
    SYS = 0x1f,
  };

  uint32_t regs[16];
  uint32_t regs_fiq[7];
  uint32_t regs_svc[2];
  uint32_t regs_abt[2];
  uint32_t regs_irq[2];
  uint32_t regs_und[2];

  uint32_t cpsr;
  uint32_t spsr_fiq;
  uint32_t spsr_svc;
  uint32_t spsr_abt;
  uint32_t spsr_irq;
  uint32_t spsr_und;

  CYCLE_TYPE cycle_type;

  uint32_t pipeline[2];
  uint64_t cycles;

  MODE get_mode();

  uint32_t get_psr();
  void set_psr(uint32_t val);

  bool get_cc(FLAG f);
  void set_cc(FLAG f, bool val);

  Bus &bus;

  ARM *arm;
};
