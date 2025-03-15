#pragma once
#include <cstdint>
#include <memory>

class Bus;

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

  CPU(std::unique_ptr<Bus> bus);

  void start(const char *rom_file, const char *bios_file);

  CYCLE_TYPE get_last_cycle_type() { return cycle_type; }

private:
  bool eval_cond(uint32_t instr);

  uint32_t get_reg(uint8_t rn);
  void set_reg(uint8_t rn, uint32_t val);

  uint32_t get_cpsr();
  void set_cpsr(uint32_t val);

  void cycle(uint64_t count);

  void arm_fetch();
  uint32_t arm_fetch_next();

  void thumb_fetch();
  void thumb_fetch_next();

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

  CYCLE_TYPE cycle_type;

  uint32_t pipeline[2];
  uint32_t cycles;

  MODE get_mode();

  uint32_t get_psr();
  void set_psr(uint32_t val);

  bool get_cc(FLAG f);
  void set_cc(FLAG f, bool val);

  void reset();

  bool running;

  void run();

  // Bus &bus;
  std::unique_ptr<Bus> bus;

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

  bool is_bx(uint32_t instr);
  bool is_bdt(uint32_t instr);
  bool is_bl(uint32_t instr);
  bool is_swi(uint32_t instr);
  bool is_und(uint32_t instr);
  bool is_sdt(uint32_t instr);
  bool is_sds(uint32_t instr);
  bool is_mul(uint32_t instr);
  bool is_hdtri(uint32_t instr);
  bool is_psrt(uint32_t instr);
  bool is_dproc(uint32_t instr);

  void bx(uint32_t instr);
  void bdt(uint32_t instr);
  void bl(uint32_t instr);
  void swi(uint32_t instr);
  void und(uint32_t instr);
  void sdt(uint32_t instr);
  void sds(uint32_t instr);
  void mul(uint32_t instr);
  void hdtri(uint32_t instr);
  void psrt(uint32_t instr);
  void dproc(uint32_t instr);
};
