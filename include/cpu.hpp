#pragma once
#include <cstdint>

class CPU {

public:
  // instr[31:28]
  enum COND_FIELD {
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

  // cspr[31:28] = N Z C V
  enum COND_CODE {
    N = 1 << 31,
    Z = 1 << 30,
    C = 1 << 29,
    V = 1 << 28,
  };

  // cspr[7:0] = I F T M[4:0]
  enum CONTROL_BITS {
    I = 1 << 7,
    F = 1 << 6,
    T = 1 << 5,
    M = 0x1f,
  };

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

  bool eval_cond(uint32_t instr);
};
