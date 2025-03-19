#include "cpu.h"
#include "bus.h"
#include <chrono>
#include <thread>

CPU::CPU() {};

CPU::~CPU() { delete bus; };

void CPU::set_bus(Bus *bus) { this->bus = bus; }

void CPU::start(const char *rom_file, const char *bios_file) {
  bus->load_bios(bios_file);
  bus->load_rom(rom_file);

  bus->update_wait();

  reset();

  running = true;

  run();
}

void CPU::run() {
  while (running) {
    // std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    auto start_time = std::chrono::steady_clock::now();
    cycles = 0;

    while (cycles < (2 << 24) && running) {
      // std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      if (cpsr & CONTROL::T) {
        uint16_t instr = thumb_fetch_next();
        // std::cout << std::hex << regs[15] - 4 << ": " << instr << std::endl;

        static constexpr struct {
          uint16_t mask;                  // Bitmask for matching
          uint16_t format;                // Expected pattern
          void (CPU::*handler)(uint16_t); // Function pointer to handler
        } thumb_handlers[] = {
            {0xF000, 0xF000, &CPU::thumb_lbl},    // lbl
            {0xF800, 0xE000, &CPU::thumb_ub},     // ub
            {0xFF00, 0xDF00, &CPU::thumb_swi},    // swi
            {0xF000, 0xD000, &CPU::thumb_cb},     // cb
            {0xF000, 0xC000, &CPU::thumb_mls},    // mls
            {0xFF00, 0xB000, &CPU::thumb_aosp},   // aosp
            {0xF000, 0xB000, &CPU::thumb_ppr},    // ppr
            {0xF000, 0xA000, &CPU::thumb_la},     // la
            {0xF800, 0x9000, &CPU::thumb_sprls},  // sprls
            {0xF800, 0x8000, &CPU::thumb_lsh},    // lsh
            {0xE000, 0x6000, &CPU::thumb_lsio},   // lsio
            {0xF200, 0x5000, &CPU::thumb_lsro},   // lsro
            {0xF200, 0x5200, &CPU::thumb_lssebh}, // lssebh
            {0xF800, 0x4800, &CPU::thumb_pcrl},   // pcrl
            {0xFC00, 0x4400, &CPU::thumb_hrobx},  // hrobx
            {0xFC00, 0x4000, &CPU::thumb_alu},    // alu
            {0xE000, 0x2000, &CPU::thumb_mcasi},  // mcasi
            {0xF800, 0x1800, &CPU::thumb_as},     // as
            {0xE000, 0x0000, &CPU::thumb_msr},    // msr
        };

        // Search for a matching instruction handler
        for (const auto &entry : thumb_handlers) {
          if ((instr & entry.mask) == entry.format) {
            (this->*entry.handler)(instr);
            break;
          }
        }
      } else {
        uint32_t instr = arm_fetch_next();

        // std::cout << std::hex << regs[15] - 8 << ": " << std::hex << instr
        // << std::endl;
        //           << ": ";
        COND cond = static_cast<COND>((instr >> 28) & 0xf);
        if (!eval_cond(cond)) {
          continue;
        }

        if (arm_is_bx(instr)) {
          // std::cout << "bx" << std::endl;
          arm_bx(instr); // NOTE: DONE
        } else if (arm_is_bdt(instr)) {
          // std::cout << "bdt" << std::endl;
          arm_bdt(instr);
        } else if (arm_is_bl(instr)) {
          // std::cout << "bl" << std::endl;
          arm_bl(instr); // NOTE: DONE
        } else if (arm_is_swi(instr)) {
          arm_swi(instr);
        } else if (arm_is_und(instr)) {
          arm_und(instr);
        } else if (arm_is_sdt(instr)) {
          arm_sdt(instr); // NOTE: DONE
        } else if (arm_is_sds(instr)) {
          NYI("sds");
          // sds(instr);
        } else if (arm_is_mul(instr)) {
          arm_mul(instr);
        } else if (arm_is_hdtri(instr)) {
          arm_hdtri(instr);
        } else if (arm_is_psrt(instr)) {
          arm_psrt(instr); // NOTE: DONE
        } else if (arm_is_dproc(instr)) {
          arm_dproc(instr); // NOTE: DONE
        } else {
          std::cout << "unknown" << std::endl;
          running = false;
        }
      }
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed_time =
        std::chrono::duration_cast<std::chrono::microseconds>(now - start_time)
            .count();
    std::cout << "Elapsed time: " << elapsed_time << std::endl;
    auto sleep_time = std::chrono::microseconds(1000000) -
                      std::chrono::microseconds(elapsed_time);

    if (sleep_time.count() > 0) {
      std::this_thread::sleep_for(sleep_time);
    }
  }
}

void CPU::reset() {
  regs[0] = regs[1] = regs[2] = regs[3] = regs[4] = regs[5] = regs[6] =
      regs[7] = regs[8] = regs[9] = regs[10] = regs[11] = regs[12] = regs[14] =
          0;

  cpsr = 0;
  bool use_bios = false;
  if (!use_bios) {
    regs[13] = regs_fiq[5] = regs_abt[0] = regs_und[0] = 0x03007F00;
    regs_svc[0] = 0x03007FE0;
    regs_irq[0] = 0x03007FA0;

    regs[15] = 0x08000000;

    cpsr |= MODE::SYS;
    cpsr |= CONTROL::F;
  } else {
    regs[13] = regs_fiq[5] = regs_abt[0] = regs_und[0] = 0;
    regs_svc[0] = 0;
    regs_irq[0] = 0;
    regs[15] = 0;

    cpsr |= MODE::SVC;
    cpsr |= CONTROL::I;
    cpsr |= CONTROL::F;
  }

  regs_fiq[0] = regs_fiq[1] = regs_fiq[2] = regs_fiq[3] = regs_fiq[4] =
      regs_fiq[6] = spsr_fiq = 0;
  regs_svc[1] = spsr_svc = 0;
  regs_abt[1] = spsr_abt = 0;
  regs_irq[1] = spsr_irq = 0;
  regs_und[1] = spsr_und = 0;

  arm_fetch();
}

uint32_t CPU::get_cpsr() { return cpsr; }
void CPU::set_cpsr(uint32_t val) { cpsr = val; }

void CPU::cycle(uint32_t count) {
  // for (uint32_t i = 0; i < count; i++) {
  //   // if (cycles % 64 == 0) {
  //   // bus->tick_ppu();
  //   // }
  // }
  bus->tick_ppu(count);
  cycles += count;
}

void CPU::arm_fetch() {
  pipeline[0] = bus->read32(regs[15], CYCLE_TYPE::NON_SEQ);
  pipeline[1] = bus->read32(regs[15] + 4, CYCLE_TYPE::SEQ);
  regs[15] += 4;
}

uint32_t CPU::arm_fetch_next() {
  uint32_t instr = pipeline[0];
  pipeline[0] = pipeline[1];
  pipeline[1] = bus->read32(regs[15] + 4, CYCLE_TYPE::SEQ);
  regs[15] += 4;
  return instr;
}

void CPU::thumb_fetch() {
  pipeline[0] = bus->read16(regs[15], CYCLE_TYPE::NON_SEQ);
  pipeline[1] = bus->read16(regs[15] + 2, CYCLE_TYPE::SEQ);
  regs[15] += 2;
}

uint16_t CPU::thumb_fetch_next() {
  uint16_t instr = pipeline[0];
  pipeline[0] = pipeline[1];
  pipeline[1] = bus->read16(regs[15] + 2, CYCLE_TYPE::SEQ);
  regs[15] += 2;
  return instr;
}

uint32_t CPU::get_reg(uint8_t reg) {
  if (reg <= 0x7 || reg == 0xF) {
    return regs[reg];
  }

  MODE mode = get_mode();

  if (reg >= 0x8 && reg <= 0xC) {
    return (mode == FIQ) ? regs_fiq[reg - 8] : regs[reg];
  }

  if (reg == 0xD || reg == 0xE) {
    if (mode == USR || mode == SYS) {
      return regs[reg];
    } else if (mode == FIQ) {
      return regs_fiq[reg - 8];
    } else if (mode == IRQ) {
      return regs_irq[reg - 0xD];
    } else if (mode == SVC) {
      return regs_svc[reg - 0xD];
    } else if (mode == ABT) {
      return regs_abt[reg - 0xD];
    } else if (mode == UND) {
      return regs_und[reg - 0xD];
    }
  }

  return 0;
}

void CPU::set_reg(uint8_t reg, uint32_t val) {
  if (reg <= 0x7) {
    regs[reg] = val;
    return;
  }

  if (reg == 0xF) {
    regs[15] = (cpsr & CONTROL::T) ? (val & ~0x1) : (val & ~0x3);
    return;
  }

  MODE mode = get_mode();

  if (reg >= 0x8 && reg <= 0xC) {
    if (mode == FIQ) {
      regs_fiq[reg - 8] = val;
    } else {
      regs[reg] = val;
    }
    return;
  }

  if (reg == 0xD || reg == 0xE) {
    if (mode == USR || mode == SYS) {
      regs[reg] = val;
    } else if (mode == FIQ) {
      regs_fiq[reg - 8] = val;
    } else if (mode == IRQ) {
      regs_irq[reg - 0xD] = val;
    } else if (mode == SVC) {
      regs_svc[reg - 0xD] = val;
    } else if (mode == ABT) {
      regs_abt[reg - 0xD] = val;
    } else if (mode == UND) {
      regs_und[reg - 0xD] = val;
    }
  }
}

// uint32_t CPU::get_psr() {
//   switch (get_mode()) {
//   case USR:
//   case SYS:
//     return cpsr;
//   case FIQ:
//     return spsr_fiq;
//   case IRQ:
//     return spsr_irq;
//   case SVC:
//     return spsr_svc;
//   case ABT:
//     return spsr_abt;
//   case UND:
//     return spsr_und;
//   default:
//     return 0;
//   }
// }
//
// void CPU::set_psr(uint32_t val) {
//   switch (get_mode()) {
//   case USR:
//   case SYS:
//     cpsr = val;
//     break;
//   case FIQ:
//     spsr_fiq = val;
//     break;
//   case IRQ:
//     spsr_irq = val;
//     break;
//   case SVC:
//     spsr_svc = val;
//     break;
//   case ABT:
//     spsr_abt = val;
//     break;
//   case UND:
//     spsr_und = val;
//     break;
//   default:
//     break;
//   }
// }

// bool CPU::get_cc(FLAG f) { return get_psr() & f; }
// void CPU::set_cc(FLAG f, bool val) {
//   uint32_t psr = get_psr();
//   if (val) {
//     psr |= f;
//   } else {
//     psr &= ~f;
//   }
//   set_psr(psr);
// }

bool CPU::eval_cond(COND cond) {
  bool n = get_cc(N);
  bool z = get_cc(Z);
  bool c = get_cc(C);
  bool v = get_cc(V);

  using CondFunc = bool (*)(bool, bool, bool, bool);

  static const CondFunc cond_eval[] = {
      [](bool, bool z, bool, bool) -> bool { return z; },                  // EQ
      [](bool, bool z, bool, bool) -> bool { return !z; },                 // NE
      [](bool, bool, bool c, bool) -> bool { return c; },                  // CS
      [](bool, bool, bool c, bool) -> bool { return !c; },                 // CC
      [](bool n, bool, bool, bool) -> bool { return n; },                  // MI
      [](bool n, bool, bool, bool) -> bool { return !n; },                 // PL
      [](bool, bool, bool, bool v) -> bool { return v; },                  // VS
      [](bool, bool, bool, bool v) -> bool { return !v; },                 // VC
      [](bool, bool z, bool c, bool) -> bool { return c && !z; },          // HI
      [](bool, bool z, bool c, bool) -> bool { return !c || z; },          // LS
      [](bool n, bool, bool, bool v) -> bool { return n == v; },           // GE
      [](bool n, bool, bool, bool v) -> bool { return n != v; },           // LT
      [](bool n, bool z, bool, bool v) -> bool { return !z && (n == v); }, // GT
      [](bool n, bool z, bool, bool v) -> bool { return z || (n != v); },  // LE
      [](bool, bool, bool, bool) -> bool { return true; }                  // AL
  };

  return (cond <= AL) ? cond_eval[cond](n, z, c, v) : false;
}
