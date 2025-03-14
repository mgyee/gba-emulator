#pragma once
#include "cpu.h"
#include <memory>

#define BIOS_START (0x00000000) // BIOS - System ROM (16 KiB)
#define BIOS_END (0x00003FFF)
// 00004000-01FFFFFF Not used
#define EWRAM_START (0x02000000) // WRAM - On-board Work RAM (256 KiB) 2 Wait
#define EWRAM_END (0x0203FFFF)
// 02040000-02FFFFFF   Not used
#define IWRAM_START (0x03000000) // WRAM - On-chip Work RAM (32 KiB)
#define IWRAM_END (0x03007FFF)
// 03008000-03FFFFFF   Not used
#define MMIO_START (0x04000000) // I/O Registers (1 KiB)
#define MMIO_END (0x040003FF)
// 04000400-04FFFFFF   Not used
#define PALRAM_START (0x05000000) // BG/OBJ Palette RAM (1 KiB)
#define PALRAM_END (0x050003FF)
// 05000400-05FFFFFF   Not used
#define VRAM_START (0x06000000) // VRAM - Video RAM (96 KiB)
#define VRAM_END (0x06017FFF)
// 06018000-06FFFFFF   Not used
#define OAM_START (0x07000000) // OAM - OBJ Attributes (1 KiB)
#define OAM_END (0x070003FF)
// 07000400-07FFFFFF   Not used
#define CART_0_START (0x08000000) // Game Pak ROM (max 32MiB) - Wait State 0
#define CART_0_END (0x09FFFFFF)
#define CART_1_START (0x0A000000) // Game Pak ROM (max 32MiB) - Wait State 1
#define CART_1_END (0x0BFFFFFF)
#define CART_2_START (0x0C000000) // Game Pak ROM (max 32MiB) - Wait State 2
#define CART_2_END (0x0DFFFFFF)
#define SRAM_START (0x0E000000) // Game Pak SRAM (max 64 KiB) - 8bit Bus width
#define SRAM_END (0x0E00FFFF)
// 0E010000-0FFFFFFF   Not used
// 10000000-FFFFFFFF   Not used

// class CPU;

class Bus {
public:
  Bus();

  void write32(uint32_t addr, uint32_t data, CPU::CYCLE_TYPE type);
  uint32_t read32(uint32_t addr, CPU::CYCLE_TYPE type);

  void write16(uint32_t addr, uint16_t data, CPU::CYCLE_TYPE type);
  uint16_t read16(uint32_t addr, CPU::CYCLE_TYPE type);

  void attach_cpu(std::unique_ptr<CPU> cpu);

  bool load_bios(const char *bios_file);
  bool load_rom(const char *rom_file);

private:
  // std::unique_ptr<CPU> cpu;

  uint8_t bios[BIOS_END - BIOS_START + 1];
  uint8_t ewram[EWRAM_END - EWRAM_START + 1];
  uint8_t iwram[IWRAM_END - IWRAM_START + 1];
  uint8_t mmio[MMIO_END - MMIO_START + 1];
  uint8_t palram[PALRAM_END - PALRAM_START + 1];
  uint8_t vram[VRAM_END - VRAM_START + 1];
  uint8_t oam[OAM_END - OAM_START + 1];
  uint8_t rom[CART_0_END - CART_0_START + 1];
  // uint8_t cart_1[CART_1_END - CART_1_START + 1];
  // uint8_t cart_2[CART_2_END - CART_2_START + 1];
  uint8_t sram[SRAM_END - SRAM_START + 1];

  uint32_t read_open_bus(uint32_t addr);
  uint32_t read_sram(uint32_t addr);
  uint32_t read_mmio(uint32_t addr);
};
