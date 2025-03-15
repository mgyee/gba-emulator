#include "bus.h"
#include <cstdio>

Bus::Bus(CPU &cpu) : cpu(cpu) {};

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
}

uint32_t Bus::read32(uint32_t addr, CPU::CYCLE_TYPE type) {
  addr &= ~0x3;
  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      return read_open_bus(addr);
    } else {
      return *reinterpret_cast<uint32_t *>(bios + addr);
    }
  case 0x02: // EWRAM
    return *reinterpret_cast<uint32_t *>(ewram + (addr & 0x3fff));
  case 0x03: // IWRAM
    return *reinterpret_cast<uint32_t *>(iwram + (addr & 0x7fff));
  case 0x04: // MMIO
    return (read8_mmio(addr + 0) << 0) | (read8_mmio(addr + 1) << 8) |
           (read8_mmio(addr + 2) << 16) | (read8_mmio(addr + 3) << 24);
  case 0x05: // PALRAM
    return *reinterpret_cast<uint32_t *>(palram + (addr & 0x3ff));
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    return *reinterpret_cast<uint32_t *>(vram + addr);
  case 0x07: // OAM
    return *reinterpret_cast<uint32_t *>(oam + (addr & 0x3ff));
  case 0x08 ... 0x0D:
    return *reinterpret_cast<uint32_t *>(rom + (addr & 0x1ffffff));
  case 0x0E ... 0x0F:
    return read_sram(addr);
  default:
    return 0;
  }
}

uint16_t Bus::read16(uint32_t addr, CPU::CYCLE_TYPE type) {
  addr &= ~0x1;
  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      return read_open_bus(addr);
    } else {
      return *reinterpret_cast<uint16_t *>(bios + addr);
    }
  case 0x02: // EWRAM
    return *reinterpret_cast<uint16_t *>(ewram + (addr & 0x3fff));
  case 0x03: // IWRAM
    return *reinterpret_cast<uint16_t *>(iwram + (addr & 0x7fff));
  case 0x04: // MMIO
    return (read8_mmio(addr + 0) << 0) | (read8_mmio(addr + 1) << 8);
  case 0x05: // PALRAM
    return *reinterpret_cast<uint16_t *>(palram + (addr & 0x3ff));
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    return *reinterpret_cast<uint16_t *>(vram + addr);
  case 0x07: // OAM
    return *reinterpret_cast<uint16_t *>(oam + (addr & 0x3ff));
  case 0x08 ... 0x0D:
    return *reinterpret_cast<uint16_t *>(rom + (addr & 0x1ffffff));
  case 0x0E ... 0x0F:
    return read_sram(addr);
  default:
    return 0;
  }
}

uint8_t Bus::read8(uint32_t addr, CPU::CYCLE_TYPE type) {

  switch ((addr >> 24) & 0xff) {
  case 0x00: // BIOS
    if (addr >= 0x4000) {
      return read_open_bus(addr);
    } else {
      return *reinterpret_cast<uint8_t *>(bios + addr);
    }
  case 0x02: // EWRAM
    return *reinterpret_cast<uint8_t *>(ewram + (addr & 0x3fff));
  case 0x03: // IWRAM
    return *reinterpret_cast<uint8_t *>(iwram + (addr & 0x7fff));
  case 0x04: // MMIO
    return (read8_mmio(addr + 0) << 0);
  case 0x05: // PALRAM
    return *reinterpret_cast<uint8_t *>(palram + (addr & 0x3ff));
  case 0x06: // VRAM
    addr &= 0x1ffff;
    if (addr >= 0x18000) {
      addr &= 0x7fff;
    }
    return *reinterpret_cast<uint8_t *>(vram + addr);
  case 0x07: // OAM
    return *reinterpret_cast<uint8_t *>(oam + (addr & 0x3ff));
  case 0x08 ... 0x0D:
    return *reinterpret_cast<uint8_t *>(rom + (addr & 0x1ffffff));
  case 0x0E ... 0x0F:
    return read_sram(addr);
  default:
    return 0;
  }
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

    lcd.dispcnt.bytes[0] = data;
    break;
  case REG_DISPCNT + 1:
    lcd.dispcnt.bytes[1] = data;
    break;
  case REG_GREENSWP:
    lcd.greenswp.bytes[0] = data;
    break;
  case REG_GREENSWP + 1:
    lcd.greenswp.bytes[1] = data;
    break;
  case REG_DISPSTAT:
    lcd.dispstat.bytes[0] &= 0x47;
    lcd.dispstat.bytes[0] |= data & ~0x47;
    break;
  case REG_DISPSTAT + 1:
    lcd.dispstat.bytes[1] = data;
    break;
  case REG_BG0CNT:
    lcd.bgcnt[0].bytes[0] = data;
    break;
  case REG_BG0CNT + 1:
    lcd.bgcnt[0].bytes[1] = data & 0xDF;
    break;
  case REG_BG1CNT:
    lcd.bgcnt[1].bytes[0] = data;
    break;
  case REG_BG1CNT + 1:
    lcd.bgcnt[1].bytes[1] = data & 0xDF;
    break;
  case REG_BG2CNT:
    lcd.bgcnt[2].bytes[0] = data;
    break;
  case REG_BG2CNT + 1:
    lcd.bgcnt[2].bytes[1] = data;
    break;
  case REG_BG3CNT:
    lcd.bgcnt[3].bytes[0] = data;
    break;
  case REG_BG3CNT + 1:
    lcd.bgcnt[3].bytes[1] = data;
    break;
  case REG_BG0HOFS:
    lcd.bghofs[0].bytes[0] = data;
    break;
  case REG_BG0HOFS + 1:
    lcd.bghofs[0].bytes[1] = data & 0x1;
    break;
  case REG_BG0VOFS:
    lcd.bgvofs[0].bytes[0] = data;
    break;
  case REG_BG0VOFS + 1:
    lcd.bgvofs[0].bytes[1] = data & 0x1;
    break;
  case REG_BG1HOFS:
    lcd.bghofs[1].bytes[0] = data;
    break;
  case REG_BG1HOFS + 1:
    lcd.bghofs[1].bytes[1] = data & 0x1;
    break;
  case REG_BG1VOFS:
    lcd.bgvofs[1].bytes[0] = data;
    break;
  case REG_BG1VOFS + 1:
    lcd.bgvofs[1].bytes[1] = data & 0x1;
    break;
  case REG_BG2HOFS:
    lcd.bghofs[2].bytes[0] = data;
    break;
  case REG_BG2HOFS + 1:
    lcd.bghofs[2].bytes[1] = data & 0x1;
    break;
  case REG_BG2VOFS:
    lcd.bgvofs[2].bytes[0] = data;
    break;
  case REG_BG2VOFS + 1:
    lcd.bgvofs[2].bytes[1] = data & 0x1;
    break;
  case REG_BG3HOFS:
    lcd.bghofs[3].bytes[0] = data;
    break;
  case REG_BG3HOFS + 1:
    lcd.bghofs[3].bytes[1] = data & 0x1;
    break;
  case REG_BG3VOFS:
    lcd.bgvofs[3].bytes[0] = data;
    break;
  case REG_BG3VOFS + 1:
    lcd.bgvofs[3].bytes[1] = data & 0x1;
    break;
  case REG_BG2PA:
    lcd.bgpa[0].bytes[0] = data;
    break;
  case REG_BG2PA + 1:
    lcd.bgpa[0].bytes[1] = data;
    break;
  case REG_BG2PB:
    lcd.bgpb[0].bytes[0] = data;
    break;
  case REG_BG2PB + 1:
    lcd.bgpb[0].bytes[1] = data;
    break;
  case REG_BG2PC:
    lcd.bgpc[0].bytes[0] = data;
    break;
  case REG_BG2PC + 1:
    lcd.bgpc[0].bytes[1] = data;
    break;
  case REG_BG2PD:
    lcd.bgpd[0].bytes[0] = data;
    break;
  case REG_BG2PD + 1:
    lcd.bgpd[0].bytes[1] = data;
    break;
  case REG_BG2X:
    internalPX[0].bytes[0] = lcd.bgx[0].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 1:
    internalPX[0].bytes[1] = lcd.bgx[0].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 2:
    internalPX[0].bytes[2] = lcd.bgx[0].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2X + 3:
    internalPX[0].bytes[3] = lcd.bgx[0].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y:
    internalPY[0].bytes[0] = lcd.bgy[0].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 1:
    internalPY[0].bytes[1] = lcd.bgy[0].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 2:
    internalPY[0].bytes[2] = lcd.bgy[0].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG2Y + 3:
    internalPY[0].bytes[3] = lcd.bgy[0].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3PA:
    lcd.bgpa[1].bytes[0] = data;
    break;
  case REG_BG3PA + 1:
    lcd.bgpa[1].bytes[1] = data;
    break;
  case REG_BG3PB:
    lcd.bgpb[1].bytes[0] = data;
    break;
  case REG_BG3PB + 1:
    lcd.bgpb[1].bytes[1] = data;
    break;
  case REG_BG3PC:
    lcd.bgpc[1].bytes[0] = data;
    break;
  case REG_BG3PC + 1:
    lcd.bgpc[1].bytes[1] = data;
    break;
  case REG_BG3PD:
    lcd.bgpd[1].bytes[0] = data;
    break;
  case REG_BG3PD + 1:
    lcd.bgpd[1].bytes[1] = data;
    break;
  case REG_BG3X:
    internalPX[1].bytes[0] = lcd.bgx[1].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 1:
    internalPX[1].bytes[1] = lcd.bgx[1].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 2:
    internalPX[1].bytes[2] = lcd.bgx[1].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3X + 3:
    internalPX[1].bytes[3] = lcd.bgx[1].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y:
    internalPY[1].bytes[0] = lcd.bgy[1].bytes[0] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 1:
    internalPY[1].bytes[1] = lcd.bgy[1].bytes[1] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 2:
    internalPY[1].bytes[2] = lcd.bgy[1].bytes[2] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_BG3Y + 3:
    internalPY[1].bytes[3] = lcd.bgy[1].bytes[3] = data;
    break; // gba->ppu.reload_internal_affine_regs = true; break;
  case REG_WIN0H:
    lcd.winh[0].bytes[0] = data;
    break;
  case REG_WIN0H + 1:
    lcd.winh[0].bytes[1] = data;
    break;
  case REG_WIN1H:
    lcd.winh[1].bytes[0] = data;
    break;
  case REG_WIN1H + 1:
    lcd.winh[1].bytes[1] = data;
    break;
  case REG_WIN0V:
    lcd.winv[0].bytes[0] = data;
    break;
  case REG_WIN0V + 1:
    lcd.winv[0].bytes[1] = data;
    break;
  case REG_WIN1V:
    lcd.winv[1].bytes[0] = data;
    break;
  case REG_WIN1V + 1:
    lcd.winv[1].bytes[1] = data;
    break;
  case REG_WININ:
    lcd.winin.bytes[0] = data & 0x3F;
    break;
  case REG_WININ + 1:
    lcd.winin.bytes[1] = data & 0x3F;
    break;
  case REG_WINOUT:
    lcd.winout.bytes[0] = data & 0x3F;
    break;
  case REG_WINOUT + 1:
    lcd.winout.bytes[1] = data & 0x3F;
    break;
  case REG_MOSAIC:
    lcd.mosaic.bytes[0] = data;
    break;
  case REG_MOSAIC + 1:
    lcd.mosaic.bytes[1] = data;
    break;
  case REG_BLDCNT:
    lcd.bldcnt.bytes[0] = data;
    break;
  case REG_BLDCNT + 1:
    lcd.bldcnt.bytes[1] = data & 0x3F;
    break;
  case REG_BLDALPHA:
    lcd.bldalpha.bytes[0] = data & 0x1F;
    break;
  case REG_BLDALPHA + 1:
    lcd.bldalpha.bytes[1] = data & 0x1F;
    break;
  case REG_BLDY:
    lcd.bldy.bytes[0] = data;
    break;
  case REG_BLDY + 1:
    lcd.bldy.bytes[1] = data;
    break;
  default:
    break;
  }
}
uint8_t Bus::read8_mmio(uint32_t addr) {
  switch (addr) {
  /* LCD I/O Registers */
  case REG_DISPCNT:
    return (lcd.dispcnt.bytes[0]);
  case REG_DISPCNT + 1:
    return (lcd.dispcnt.bytes[1]);
  case REG_GREENSWP:
    return (lcd.greenswp.bytes[0]);
  case REG_GREENSWP + 1:
    return (lcd.greenswp.bytes[1]);
  case REG_DISPSTAT:
    return (lcd.dispstat.bytes[0]);
  case REG_DISPSTAT + 1:
    return (lcd.dispstat.bytes[1]);
  case REG_VCOUNT:
    return (lcd.vcount.bytes[0]);
  case REG_VCOUNT + 1:
    return (lcd.vcount.bytes[1]);
  case REG_BG0CNT:
    return (lcd.bgcnt[0].bytes[0]);
  case REG_BG0CNT + 1:
    return (lcd.bgcnt[0].bytes[1]);
  case REG_BG1CNT:
    return (lcd.bgcnt[1].bytes[0]);
  case REG_BG1CNT + 1:
    return (lcd.bgcnt[1].bytes[1]);
  case REG_BG2CNT:
    return (lcd.bgcnt[2].bytes[0]);
  case REG_BG2CNT + 1:
    return (lcd.bgcnt[2].bytes[1]);
  case REG_BG3CNT:
    return (lcd.bgcnt[3].bytes[0]);
  case REG_BG3CNT + 1:
    return (lcd.bgcnt[3].bytes[1]);
  case REG_WININ:
    return (lcd.winin.bytes[0]);
  case REG_WININ + 1:
    return (lcd.winin.bytes[1]);
  case REG_WINOUT:
    return (lcd.winout.bytes[0]);
  case REG_WINOUT + 1:
    return (lcd.winout.bytes[1]);
  case REG_BLDCNT:
    return (lcd.bldcnt.bytes[0]);
  case REG_BLDCNT + 1:
    return (lcd.bldcnt.bytes[1]);
  case REG_BLDALPHA:
    return (lcd.bldalpha.bytes[0]);
  case REG_BLDALPHA + 1:
    return (lcd.bldalpha.bytes[1]);
  default:
    return 0;
  }
}
