#pragma once

#include "cpu.h"
#include <cstdint>
class ARM {
private:
  enum INSTR_TYPE {
    BX,
    BDT,
    BL,
    SWI,
    UND,
    SDT,
    SDS,
    MUL,
    HDTR,
    HDTI,
    PMRS,
    PMSR,
    DPROC,
    // TSWI,
    // UB,
    // CB,
    // MLS,
    // LBL,
    // AOSP,
    // PPR,
    // LSH,
    // SPRLS,
    // LA,
    // LSIO,
    // LSRO,
    // LSSEBH,
    // PCRL,
    // HROBX,
    // ALU,
    // MCASI,
    // AS,
    // MSR
  };

  CPU *cpu;

  bool is_bx(uint32_t instr);
  bool is_bdt(uint32_t instr);
  bool is_bl(uint32_t instr);
  bool is_swi(uint32_t instr);
  bool is_und(uint32_t instr);
  bool is_sdt(uint32_t instr);
  bool is_sds(uint32_t instr);
  bool is_mul(uint32_t instr);
  bool is_hdtr(uint32_t instr);
  bool is_hdti(uint32_t instr);
  bool is_pmrs(uint32_t instr);
  bool is_pmsr(uint32_t instr);
  bool is_dproc(uint32_t instr);

  void bx(uint32_t instr);
  void bdt(uint32_t instr);
  void bl(uint32_t instr);
  void swi(uint32_t instr);
  void und(uint32_t instr);
  void sdt(uint32_t instr);
  void sds(uint32_t instr);
  void mul(uint32_t instr);
  void hdtr(uint32_t instr);
  void hdti(uint32_t instr);
  void pmrs(uint32_t instr);
  void pmsr(uint32_t instr);
  void dproc(uint32_t instr);

  uint32_t arm_execute();
};
