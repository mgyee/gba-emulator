#include "bus.h"
#include <cstdio>

Bus::Bus() {};

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

// void Bus::attach_cpu(std::unique_ptr<CPU> cpu) { this->cpu = std::move(cpu);
// }

uint32_t Bus::read32(uint32_t addr, CPU::CYCLE_TYPE type) {
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
    return read_mmio(addr);
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

uint32_t Bus::read_open_bus(uint32_t addr) { return 0; }

uint32_t Bus::read_sram(uint32_t addr) { return 0; }

uint32_t Bus::read_mmio(uint32_t addr) { return 0; }
