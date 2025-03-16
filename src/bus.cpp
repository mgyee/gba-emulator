#include "bus.h"
#include <cstdio>

Bus::Bus(CPU &cpu) : cpu(cpu) {};

Bus::~Bus() { delete ppu; }

void Bus::attach_ppu(PPU *ppu) { this->ppu = ppu; }
void Bus::tick_ppu() { ppu->tick(); }

bool Bus::load_bios(const char *bios_file) {
  FILE *fp = fopen(bios_file, "rb");
  if (!fp) {
    return false;
  }

  if (fread(bios, sizeof(uint8_t), sizeof(bios), fp)) {
    fclose(fp);
    return false;
  }

  if (fclose(fp)) {
    return false;
  }

  return true;
}

bool Bus::load_rom(const char *rom_file) {
  FILE *fp = fopen(rom_file, "rb");
  if (!fp) {
    return false;
  }

  fseek(fp, 0, SEEK_END);

  uint64_t file_size = ftell(fp);

  fseek(fp, 0, SEEK_SET);

  if (fread(rom, sizeof(uint8_t), file_size, fp)) {
    fclose(fp);
    return false;
  }

  if (fclose(fp)) {
    return false;
  }

  return true;
}

void Bus::set_last_cycle_type(CPU::CYCLE_TYPE cycle_type) {
  last_cycle_type = cycle_type;
}

void Bus::update_wait() {
  static constexpr int nseq[4] = {4, 3, 2, 8};
  static constexpr int seq0[2] = {2, 1};
  static constexpr int seq1[2] = {4, 1};
  static constexpr int seq2[2] = {8, 1};

  bool n = CPU::CYCLE_TYPE::NON_SEQ;
  bool s = CPU::CYCLE_TYPE::SEQ;

  // Update non-sequential access times for 16-bit memory operations
  wait16[n][(CART_0_START >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws01];
  wait16[n][(CART_0_END >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws01];
  wait16[n][(CART_1_START >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws11];
  wait16[n][(CART_1_END >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws11];
  wait16[n][(CART_2_START >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws21];
  wait16[n][(CART_2_END >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.ws21];

  wait16[n][(SRAM_START >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.sram];
  // wait16[n][(SRAM_END >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.sram];

  // Update sequential access times for 16-bit memory operations
  wait16[s][(CART_0_START >> 24)] = 1 + seq0[iwpdc.waitcnt.bits.ws02];
  wait16[s][(CART_0_END >> 24)] = 1 + seq0[iwpdc.waitcnt.bits.ws02];
  wait16[s][(CART_1_START >> 24)] = 1 + seq1[iwpdc.waitcnt.bits.ws12];
  wait16[s][(CART_1_END >> 24)] = 1 + seq1[iwpdc.waitcnt.bits.ws12];
  wait16[s][(CART_2_START >> 24)] = 1 + seq2[iwpdc.waitcnt.bits.ws22];
  wait16[s][(CART_2_END >> 24)] = 1 + seq2[iwpdc.waitcnt.bits.ws22];

  wait16[s][(SRAM_START >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.sram];
  // wait16[s][(SRAM_END >> 24)] = 1 + nseq[iwpdc.waitcnt.bits.sram];

  // Update access times for 32-bit memory operations
  for (uint32_t x = (CART_0_START >> 24); x <= (SRAM_START >> 24); ++x) {
    wait32[n][x] = wait16[n][x] + wait16[s][x];
    wait32[s][x] = 2 * wait16[s][x];
  }
}

void Bus::write32(uint32_t addr, uint32_t data, CPU::CYCLE_TYPE type) {
  addr &= ~0x3;
  switch ((addr >> 24) & 0xff) {
  case 0x02:
    *reinterpret_cast<uint32_t *>(ewram + (addr & 0x3fff)) = data;
    break;
  case 0x03:
    *reinterpret_cast<uint32_t *>(iwram + (addr & 0x7fff)) = data;
    break;
  case 0x04:
    write8_mmio(addr + 0, ((data) >> 0) & 0xff);
    write8_mmio(addr + 1, ((data) >> 8) & 0xff);
    write8_mmio(addr + 2, ((data) >> 16) & 0xff);
    write8_mmio(addr + 3, ((data) >> 24) & 0xff);
    break;
  case 0x05:
    *reinterpret_cast<uint32_t *>(palram + (addr & 0x3ff)) = data;
    break;
  case 0x06:
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    *reinterpret_cast<uint32_t *>(vram + addr) = data;
    break;
  case 0x07:
    *reinterpret_cast<uint32_t *>(oam + (addr & 0x3ff)) = data;
    break;
  case 0x0C ... 0x0D:
    break;
  case 0x0E ... 0x0F:
    break;
  default:
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait32[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
}

void Bus::write16(uint32_t addr, uint16_t data, CPU::CYCLE_TYPE type) {
  addr &= ~0x1;
  switch ((addr >> 24) & 0xff) {
  case 0x02:
    *reinterpret_cast<uint16_t *>(ewram + (addr & 0x3fff)) = data;
    break;
  case 0x03:
    *reinterpret_cast<uint16_t *>(iwram + (addr & 0x7fff)) = data;
    break;
  case 0x04:
    write8_mmio(addr + 0, ((data) >> 0) & 0xff);
    write8_mmio(addr + 1, ((data) >> 8) & 0xff);
    break;
  case 0x05:
    *reinterpret_cast<uint16_t *>(palram + (addr & 0x3ff)) = data;
    break;
  case 0x06:
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    *reinterpret_cast<uint16_t *>(vram + addr) = data;
    break;
  case 0x07:
    *reinterpret_cast<uint16_t *>(oam + (addr & 0x3ff)) = data;
    break;
  default:
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait16[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
}

void Bus::write8(uint32_t addr, uint8_t data, CPU::CYCLE_TYPE type) {
  switch ((addr >> 24) & 0xff) {
  case 0x02:
    *reinterpret_cast<uint8_t *>(ewram + (addr & 0x3fff)) = data;
    break;
  case 0x03:
    *reinterpret_cast<uint8_t *>(iwram + (addr & 0x7fff)) = data;
    break;
  case 0x04:
    write8_mmio(addr + 0, ((data) >> 0) & 0xff);
    break;
  case 0x05:
    *reinterpret_cast<uint8_t *>(palram + (addr & 0x3ff)) = data;
    break;
  case 0x06:
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    *reinterpret_cast<uint8_t *>(vram + addr) = data;
    break;
  case 0x07:
    *reinterpret_cast<uint8_t *>(oam + (addr & 0x3ff)) = data;
    break;
  default:
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait16[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
}

uint32_t Bus::read32(uint32_t addr, CPU::CYCLE_TYPE type) {
  addr &= ~0x3;
  uint32_t data = 0;
  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      data = read_open_bus(addr);
    } else {
      data = *reinterpret_cast<uint32_t *>(bios + addr);
    }
    break;
  case 0x02: // EWRAM
    data = *reinterpret_cast<uint32_t *>(ewram + (addr & 0x3fff));
    break;
  case 0x03: // IWRAM
    data = *reinterpret_cast<uint32_t *>(iwram + (addr & 0x7fff));
    break;
  case 0x04: // MMIO
    data = (read8_mmio(addr + 0) << 0) | (read8_mmio(addr + 1) << 8) |
           (read8_mmio(addr + 2) << 16) | (read8_mmio(addr + 3) << 24);
    break;
  case 0x05: // PALRAM
    data = *reinterpret_cast<uint32_t *>(palram + (addr & 0x3ff));
    break;
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    data = *reinterpret_cast<uint32_t *>(vram + addr);
    break;
  case 0x07: // OAM
    data = *reinterpret_cast<uint32_t *>(oam + (addr & 0x3ff));
    break;
  case 0x08 ... 0x0D:
    data = *reinterpret_cast<uint32_t *>(rom + (addr & 0x1ffffff));
    break;
  case 0x0E ... 0x0F:
    data = read_sram(addr);
    break;
  default:
    data = 0;
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return data;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait32[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
  return data;
}

uint16_t Bus::read16(uint32_t addr, CPU::CYCLE_TYPE type) {
  addr &= ~0x1;
  uint16_t data = 0;
  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      data = read_open_bus(addr);
    } else {
      data = *reinterpret_cast<uint16_t *>(bios + addr);
    }
    break;
  case 0x02: // EWRAM
    data = *reinterpret_cast<uint16_t *>(ewram + (addr & 0x3fff));
    break;
  case 0x03: // IWRAM
    data = *reinterpret_cast<uint16_t *>(iwram + (addr & 0x7fff));
    break;
  case 0x04: // MMIO
    data = (read8_mmio(addr + 0) << 0) | (read8_mmio(addr + 1) << 8);
    break;
  case 0x05: // PALRAM
    data = *reinterpret_cast<uint16_t *>(palram + (addr & 0x3ff));
    break;
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    data = *reinterpret_cast<uint16_t *>(vram + addr);
    break;
  case 0x07: // OAM
    data = *reinterpret_cast<uint16_t *>(oam + (addr & 0x3ff));
    break;
  case 0x08 ... 0x0D:
    data = *reinterpret_cast<uint16_t *>(rom + (addr & 0x1ffffff));
    break;
  case 0x0E ... 0x0F:
    data = read_sram(addr);
    break;
  default:
    data = 0;
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return data;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait16[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
  return data;
}

uint8_t Bus::read8(uint32_t addr, CPU::CYCLE_TYPE type) {
  uint8_t data = 0;
  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      data = read_open_bus(addr);
    } else {
      data = *reinterpret_cast<uint8_t *>(bios + addr);
    }
    break;
  case 0x02: // EWRAM
    data = *reinterpret_cast<uint8_t *>(ewram + (addr & 0x3fff));
    break;
  case 0x03: // IWRAM
    data = *reinterpret_cast<uint8_t *>(iwram + (addr & 0x7fff));
    break;
  case 0x04: // MMIO
    data = (read8_mmio(addr + 0) << 0);
    break;
  case 0x05: // PALRAM
    data = *reinterpret_cast<uint8_t *>(palram + (addr & 0x3ff));
    break;
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    data = *reinterpret_cast<uint8_t *>(vram + addr);
    break;
  case 0x07: // OAM
    data = *reinterpret_cast<uint8_t *>(oam + (addr & 0x3ff));
    break;
  case 0x08 ... 0x0D:
    data = *reinterpret_cast<uint8_t *>(rom + (addr & 0x1ffffff));
    break;
  case 0x0E ... 0x0F:
    data = read_sram(addr);
    break;
  default:
    data = 0;
    break;
  }
  if (type == CPU::CYCLE_TYPE::FAST) {
    return data;
  }
  if (last_cycle_type == CPU::CYCLE_TYPE::NON_SEQ) {
    type = CPU::CYCLE_TYPE::NON_SEQ;
  }
  cpu.cycle(wait16[type][(addr >> 24)]);
  last_cycle_type = CPU::CYCLE_TYPE::SEQ;
  return data;
}

uint32_t Bus::read_open_bus(uint32_t addr) { return 0; }

uint32_t Bus::read_sram(uint32_t addr) { return 0; }

void Bus::write8_mmio(uint32_t addr, uint8_t data) {
  switch (addr) {
  /* LCD I/O Registers */
  case REG_DISPCNT:
    if (cpu.get_reg(15) >= 0x4000) {
      // The CGB mode enable bit 3 can only be set by the bios
      data &= 0xf7;
    }

    ppu->lcd.dispcnt.bytes[0] = data;
    break;
  case REG_DISPCNT + 1:
    ppu->lcd.dispcnt.bytes[1] = data;
    break;
  case REG_GREENSWP:
    ppu->lcd.greenswp.bytes[0] = data;
    break;
  case REG_GREENSWP + 1:
    ppu->lcd.greenswp.bytes[1] = data;
    break;
  case REG_DISPSTAT:
    ppu->lcd.dispstat.bytes[0] &= 0x47;
    ppu->lcd.dispstat.bytes[0] |= data & ~0x47;
    break;
  case REG_DISPSTAT + 1:
    ppu->lcd.dispstat.bytes[1] = data;
    break;
  case REG_BG0CNT:
    ppu->lcd.bgcnt[0].bytes[0] = data;
    break;
  case REG_BG0CNT + 1:
    ppu->lcd.bgcnt[0].bytes[1] = data & 0xDF;
    break;
  case REG_BG1CNT:
    ppu->lcd.bgcnt[1].bytes[0] = data;
    break;
  case REG_BG1CNT + 1:
    ppu->lcd.bgcnt[1].bytes[1] = data & 0xDF;
    break;
  case REG_BG2CNT:
    ppu->lcd.bgcnt[2].bytes[0] = data;
    break;
  case REG_BG2CNT + 1:
    ppu->lcd.bgcnt[2].bytes[1] = data;
    break;
  case REG_BG3CNT:
    ppu->lcd.bgcnt[3].bytes[0] = data;
    break;
  case REG_BG3CNT + 1:
    ppu->lcd.bgcnt[3].bytes[1] = data;
    break;
  case REG_BG0HOFS:
    ppu->lcd.bghofs[0].bytes[0] = data;
    break;
  case REG_BG0HOFS + 1:
    ppu->lcd.bghofs[0].bytes[1] = data & 0x1;
    break;
  case REG_BG0VOFS:
    ppu->lcd.bgvofs[0].bytes[0] = data;
    break;
  case REG_BG0VOFS + 1:
    ppu->lcd.bgvofs[0].bytes[1] = data & 0x1;
    break;
  case REG_BG1HOFS:
    ppu->lcd.bghofs[1].bytes[0] = data;
    break;
  case REG_BG1HOFS + 1:
    ppu->lcd.bghofs[1].bytes[1] = data & 0x1;
    break;
  case REG_BG1VOFS:
    ppu->lcd.bgvofs[1].bytes[0] = data;
    break;
  case REG_BG1VOFS + 1:
    ppu->lcd.bgvofs[1].bytes[1] = data & 0x1;
    break;
  case REG_BG2HOFS:
    ppu->lcd.bghofs[2].bytes[0] = data;
    break;
  case REG_BG2HOFS + 1:
    ppu->lcd.bghofs[2].bytes[1] = data & 0x1;
    break;
  case REG_BG2VOFS:
    ppu->lcd.bgvofs[2].bytes[0] = data;
    break;
  case REG_BG2VOFS + 1:
    ppu->lcd.bgvofs[2].bytes[1] = data & 0x1;
    break;
  case REG_BG3HOFS:
    ppu->lcd.bghofs[3].bytes[0] = data;
    break;
  case REG_BG3HOFS + 1:
    ppu->lcd.bghofs[3].bytes[1] = data & 0x1;
    break;
  case REG_BG3VOFS:
    ppu->lcd.bgvofs[3].bytes[0] = data;
    break;
  case REG_BG3VOFS + 1:
    ppu->lcd.bgvofs[3].bytes[1] = data & 0x1;
    break;
  case REG_BG2PA:
    ppu->lcd.bgpa[0].bytes[0] = data;
    break;
  case REG_BG2PA + 1:
    ppu->lcd.bgpa[0].bytes[1] = data;
    break;
  case REG_BG2PB:
    ppu->lcd.bgpb[0].bytes[0] = data;
    break;
  case REG_BG2PB + 1:
    ppu->lcd.bgpb[0].bytes[1] = data;
    break;
  case REG_BG2PC:
    ppu->lcd.bgpc[0].bytes[0] = data;
    break;
  case REG_BG2PC + 1:
    ppu->lcd.bgpc[0].bytes[1] = data;
    break;
  case REG_BG2PD:
    ppu->lcd.bgpd[0].bytes[0] = data;
    break;
  case REG_BG2PD + 1:
    ppu->lcd.bgpd[0].bytes[1] = data;
    break;
  case REG_BG2X:
    internalPX[0].bytes[0] = ppu->lcd.bgx[0].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 1:
    internalPX[0].bytes[1] = ppu->lcd.bgx[0].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 2:
    internalPX[0].bytes[2] = ppu->lcd.bgx[0].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 3:
    internalPX[0].bytes[3] = ppu->lcd.bgx[0].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y:
    internalPY[0].bytes[0] = ppu->lcd.bgy[0].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 1:
    internalPY[0].bytes[1] = ppu->lcd.bgy[0].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 2:
    internalPY[0].bytes[2] = ppu->lcd.bgy[0].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 3:
    internalPY[0].bytes[3] = ppu->lcd.bgy[0].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3PA:
    ppu->lcd.bgpa[1].bytes[0] = data;
    break;
  case REG_BG3PA + 1:
    ppu->lcd.bgpa[1].bytes[1] = data;
    break;
  case REG_BG3PB:
    ppu->lcd.bgpb[1].bytes[0] = data;
    break;
  case REG_BG3PB + 1:
    ppu->lcd.bgpb[1].bytes[1] = data;
    break;
  case REG_BG3PC:
    ppu->lcd.bgpc[1].bytes[0] = data;
    break;
  case REG_BG3PC + 1:
    ppu->lcd.bgpc[1].bytes[1] = data;
    break;
  case REG_BG3PD:
    ppu->lcd.bgpd[1].bytes[0] = data;
    break;
  case REG_BG3PD + 1:
    ppu->lcd.bgpd[1].bytes[1] = data;
    break;
  case REG_BG3X:
    internalPX[1].bytes[0] = ppu->lcd.bgx[1].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 1:
    internalPX[1].bytes[1] = ppu->lcd.bgx[1].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 2:
    internalPX[1].bytes[2] = ppu->lcd.bgx[1].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 3:
    internalPX[1].bytes[3] = ppu->lcd.bgx[1].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y:
    internalPY[1].bytes[0] = ppu->lcd.bgy[1].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 1:
    internalPY[1].bytes[1] = ppu->lcd.bgy[1].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 2:
    internalPY[1].bytes[2] = ppu->lcd.bgy[1].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 3:
    internalPY[1].bytes[3] = ppu->lcd.bgy[1].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_WIN0H:
    ppu->lcd.winh[0].bytes[0] = data;
    break;
  case REG_WIN0H + 1:
    ppu->lcd.winh[0].bytes[1] = data;
    break;
  case REG_WIN1H:
    ppu->lcd.winh[1].bytes[0] = data;
    break;
  case REG_WIN1H + 1:
    ppu->lcd.winh[1].bytes[1] = data;
    break;
  case REG_WIN0V:
    ppu->lcd.winv[0].bytes[0] = data;
    break;
  case REG_WIN0V + 1:
    ppu->lcd.winv[0].bytes[1] = data;
    break;
  case REG_WIN1V:
    ppu->lcd.winv[1].bytes[0] = data;
    break;
  case REG_WIN1V + 1:
    ppu->lcd.winv[1].bytes[1] = data;
    break;
  case REG_WININ:
    ppu->lcd.winin.bytes[0] = data & 0x3F;
    break;
  case REG_WININ + 1:
    ppu->lcd.winin.bytes[1] = data & 0x3F;
    break;
  case REG_WINOUT:
    ppu->lcd.winout.bytes[0] = data & 0x3F;
    break;
  case REG_WINOUT + 1:
    ppu->lcd.winout.bytes[1] = data & 0x3F;
    break;
  case REG_MOSAIC:
    ppu->lcd.mosaic.bytes[0] = data;
    break;
  case REG_MOSAIC + 1:
    ppu->lcd.mosaic.bytes[1] = data;
    break;
  case REG_BLDCNT:
    ppu->lcd.bldcnt.bytes[0] = data;
    break;
  case REG_BLDCNT + 1:
    ppu->lcd.bldcnt.bytes[1] = data & 0x3F;
    break;
  case REG_BLDALPHA:
    ppu->lcd.bldalpha.bytes[0] = data & 0x1F;
    break;
  case REG_BLDALPHA + 1:
    ppu->lcd.bldalpha.bytes[1] = data & 0x1F;
    break;
  case REG_BLDY:
    ppu->lcd.bldy.bytes[0] = data;
    break;
  case REG_BLDY + 1:
    ppu->lcd.bldy.bytes[1] = data;
    break;
  default:
    break;
  }
}
uint8_t Bus::read8_mmio(uint32_t addr) {
  switch (addr) {
  /* LCD I/O Registers */
  case REG_DISPCNT:
    return (ppu->lcd.dispcnt.bytes[0]);
  case REG_DISPCNT + 1:
    return (ppu->lcd.dispcnt.bytes[1]);
  case REG_GREENSWP:
    return (ppu->lcd.greenswp.bytes[0]);
  case REG_GREENSWP + 1:
    return (ppu->lcd.greenswp.bytes[1]);
  case REG_DISPSTAT:
    return (ppu->lcd.dispstat.bytes[0]);
  case REG_DISPSTAT + 1:
    return (ppu->lcd.dispstat.bytes[1]);
  case REG_VCOUNT:
    return (ppu->lcd.vcount.bytes[0]);
  case REG_VCOUNT + 1:
    return (ppu->lcd.vcount.bytes[1]);
  case REG_BG0CNT:
    return (ppu->lcd.bgcnt[0].bytes[0]);
  case REG_BG0CNT + 1:
    return (ppu->lcd.bgcnt[0].bytes[1]);
  case REG_BG1CNT:
    return (ppu->lcd.bgcnt[1].bytes[0]);
  case REG_BG1CNT + 1:
    return (ppu->lcd.bgcnt[1].bytes[1]);
  case REG_BG2CNT:
    return (ppu->lcd.bgcnt[2].bytes[0]);
  case REG_BG2CNT + 1:
    return (ppu->lcd.bgcnt[2].bytes[1]);
  case REG_BG3CNT:
    return (ppu->lcd.bgcnt[3].bytes[0]);
  case REG_BG3CNT + 1:
    return (ppu->lcd.bgcnt[3].bytes[1]);
  case REG_WININ:
    return (ppu->lcd.winin.bytes[0]);
  case REG_WININ + 1:
    return (ppu->lcd.winin.bytes[1]);
  case REG_WINOUT:
    return (ppu->lcd.winout.bytes[0]);
  case REG_WINOUT + 1:
    return (ppu->lcd.winout.bytes[1]);
  case REG_BLDCNT:
    return (ppu->lcd.bldcnt.bytes[0]);
  case REG_BLDCNT + 1:
    return (ppu->lcd.bldcnt.bytes[1]);
  case REG_BLDALPHA:
    return (ppu->lcd.bldalpha.bytes[0]);
  case REG_BLDALPHA + 1:
    return (ppu->lcd.bldalpha.bytes[1]);
  default:
    return 0;
  }
}
