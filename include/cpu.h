#pragma once
#include <cstdint>
#include <iostream>

#define NYI(str)                                                               \
  std::cout << "NYI: " << str << std::endl;                                    \
  running = false;                                                             \
  return;

class Bus;

class CPU {

public:
  enum CYCLE_TYPE {
    NON_SEQ,
    SEQ,
    FAST,
  };

  CPU();
  ~CPU();

  void set_bus(Bus *bus);

  void start(const char *rom_file, const char *bios_file);

  uint32_t get_reg(uint8_t rn);
  void set_reg(uint8_t rn, uint32_t val);

  void cycle(uint32_t count);
  // inline void CPU::cycle(uint32_t count) {
  //   for (uint32_t i = 0; i < count; i++) {
  //     // if (cycles % 64 == 0) {
  //     bus->tick_ppu();
  //     // }
  //     cycles++;
  //   }
  // }

private:
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

  bool eval_cond(COND cond);

  uint32_t get_cpsr();
  void set_cpsr(uint32_t val);

  void arm_fetch();
  uint32_t arm_fetch_next();

  void thumb_fetch();
  uint16_t thumb_fetch_next();

  void step(uint64_t count);

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

  uint32_t pipeline[2];
  uint32_t cycles;

  inline MODE get_mode() { return static_cast<MODE>(cpsr & CONTROL::M); }
  inline void set_mode(MODE mode) {
    cpsr &= ~CONTROL::M;
    cpsr |= mode;
  }

  inline uint32_t get_psr() {
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

  inline void set_psr(uint32_t val) {
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

  // uint32_t get_psr();
  // void set_psr(uint32_t val);

  inline bool get_cc(FLAG f) { return get_psr() & f; }
  inline void set_cc(FLAG f, bool val) {
    uint32_t psr = get_psr();
    if (val) {
      psr |= f;
    } else {
      psr &= ~f;
    }
    set_psr(psr);
  }

  // bool get_cc(FLAG f);
  // void set_cc(FLAG f, bool val);

  void reset();

  bool running;

  void run();

  Bus *bus;

  // std::unique_ptr<ARM> arm;
  // std::unique_ptr<Thumb> thumb;

  // ARM
  // enum INSTR_TYPE {
  //   ARM_BX,
  //   ARM_BDT,
  //   ARM_BL,
  //   ARM_SWI,
  //   ARM_UND,
  //   ARM_SDT,
  //   ARM_SDS,
  //   ARM_MUL,
  //   ARM_HDTR,
  //   ARM_HDTI,
  //   ARM_PMRS,
  //   ARM_PMSR,
  //   ARM_DPROC,
  // };
  //
  enum DPROC_OPCODE {
    AND = 0,
    EOR = 1,
    SUB = 2,
    RSB = 3,
    ADD = 4,
    ADC = 5,
    SBC = 6,
    RSC = 7,
    TST = 8,
    TEQ = 9,
    CMP = 10,
    CMN = 11,
    ORR = 12,
    MOV = 13,
    BIC = 14,
    MVN = 15,
  };

  enum SHIFT {
    LSL = 0,
    LSR = 1,
    ASR = 2,
    ROR = 3,
  };

  bool barrel_shift(uint32_t &val, SHIFT shift_type, uint8_t shift_amount,
                    bool shift_by_reg);

  // ARM instructions
  bool arm_is_bx(uint32_t instr);
  bool arm_is_bdt(uint32_t instr);
  bool arm_is_bl(uint32_t instr);
  bool arm_is_swi(uint32_t instr);
  bool arm_is_und(uint32_t instr);
  bool arm_is_sdt(uint32_t instr);
  bool arm_is_sds(uint32_t instr);
  bool arm_is_mul(uint32_t instr);
  bool arm_is_hdtri(uint32_t instr);
  bool arm_is_psrt(uint32_t instr);
  bool arm_is_dproc(uint32_t instr);

  void arm_bx(uint32_t instr);
  void arm_bdt(uint32_t instr);
  void arm_bl(uint32_t instr);
  void arm_swi(uint32_t instr);
  void arm_und(uint32_t instr);
  void arm_sdt(uint32_t instr);
  void arm_sds(uint32_t instr);
  void arm_mul(uint32_t instr);
  void arm_hdtri(uint32_t instr);
  void arm_psrt(uint32_t instr);
  void arm_dproc(uint32_t instr);

  // Thumb instructions
  void thumb_msr(uint16_t instr);
  void thumb_as(uint16_t instr);
  void thumb_mcasi(uint16_t instr);
  void thumb_alu(uint16_t instr);
  void thumb_hrobx(uint16_t instr);
  void thumb_pcrl(uint16_t instr);
  void thumb_lsro(uint16_t instr);
  void thumb_lssebh(uint16_t instr);
  void thumb_lsio(uint16_t instr);
  void thumb_lsh(uint16_t instr);
  void thumb_sprls(uint16_t instr);
  void thumb_la(uint16_t instr);
  void thumb_aosp(uint16_t instr);
  void thumb_ppr(uint16_t instr);
  void thumb_mls(uint16_t instr);
  void thumb_cb(uint16_t instr);
  void thumb_swi(uint16_t instr);
  void thumb_ub(uint16_t instr);
  void thumb_lbl(uint16_t instr);
};
