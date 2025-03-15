#pragma once
#include "cpu.h"
#include "mmio.h"

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
  Bus(CPU &cpu);

  void write32(uint32_t addr, uint32_t data, CPU::CYCLE_TYPE type);
  uint32_t read32(uint32_t addr, CPU::CYCLE_TYPE type);

  void write16(uint32_t addr, uint16_t data, CPU::CYCLE_TYPE type);
  uint16_t read16(uint32_t addr, CPU::CYCLE_TYPE type);

  void write8(uint32_t addr, uint8_t data, CPU::CYCLE_TYPE type);
  uint8_t read8(uint32_t addr, CPU::CYCLE_TYPE type);

  bool load_bios(const char *bios_file);
  bool load_rom(const char *rom_file);

private:
  // std::unique_ptr<CPU> cpu;
  CPU &cpu;

  enum IO_REGS {
    /* LCD I/O Registers */
    REG_DISPCNT = 0x04000000,  // LCD Control
    REG_GREENSWP = 0x04000002, // Undocumented - Green Swap
    REG_DISPSTAT = 0x04000004, // General LCD Status (STAT,LYC)
    REG_VCOUNT = 0x04000006,   // Vertical Counter (LY)
    REG_BG0CNT = 0x04000008,   // BG0 Control
    REG_BG1CNT = 0x0400000A,   // BG1 Control
    REG_BG2CNT = 0x0400000C,   // BG2 Control
    REG_BG3CNT = 0x0400000E,   // BG3 Control
    REG_BG0HOFS = 0x04000010,  // BG0 X-Offset
    REG_BG0VOFS = 0x04000012,  // BG0 Y-Offset
    REG_BG1HOFS = 0x04000014,  // BG1 X-Offset
    REG_BG1VOFS = 0x04000016,  // BG1 Y-Offset
    REG_BG2HOFS = 0x04000018,  // BG2 X-Offset
    REG_BG2VOFS = 0x0400001A,  // BG2 Y-Offset
    REG_BG3HOFS = 0x0400001C,  // BG3 X-Offset
    REG_BG3VOFS = 0x0400001E,  // BG3 Y-Offset
    REG_BG2PA = 0x04000020,    // BG2 Rotation/Scaling Parameter A (dx)
    REG_BG2PB = 0x04000022,    // BG2 Rotation/Scaling Parameter B (dmx)
    REG_BG2PC = 0x04000024,    // BG2 Rotation/Scaling Parameter C (dy)
    REG_BG2PD = 0x04000026,    // BG2 Rotation/Scaling Parameter D (dmy)
    REG_BG2X = 0x04000028,     // BG2 Reference Point X-Coordinate
    REG_BG2Y = 0x0400002C,     // BG2 Reference Point Y-Coordinate
    REG_BG3PA = 0x04000030,    // BG3 Rotation/Scaling Parameter A (dx)
    REG_BG3PB = 0x04000032,    // BG3 Rotation/Scaling Parameter B (dmx)
    REG_BG3PC = 0x04000034,    // BG3 Rotation/Scaling Parameter C (dy)
    REG_BG3PD = 0x04000036,    // BG3 Rotation/Scaling Parameter D (dmy)
    REG_BG3X = 0x04000038,     // BG3 Reference Point X-Coordinate
    REG_BG3Y = 0x0400003C,     // BG3 Reference Point Y-Coordinate
    REG_WIN0H = 0x04000040,    // Window 0 Horizontal Dimensions
    REG_WIN1H = 0x04000042,    // Window 1 Horizontal Dimensions
    REG_WIN0V = 0x04000044,    // Window 0 Vertical Dimensions
    REG_WIN1V = 0x04000046,    // Window 1 Vertical Dimensions
    REG_WININ = 0x04000048,    // Inside of Window 0 and 1
    REG_WINOUT = 0x0400004A,   // Inside of OBJ Window & Outside of Windows
    REG_MOSAIC = 0x0400004C,   // Mosaic Size
    // 400004Eh       -    -         Not used
    REG_BLDCNT = 0x04000050,   // Color Special Effects Selection
    REG_BLDALPHA = 0x04000052, // Alpha Blending Coefficients
    REG_BLDY = 0x04000054,     // Brightness (Fade-In/Out) Coefficient
    // 4000056h       -    -         Not used

    /* Sound Registers */
    REG_SOUND1CNT_L = 0x04000060, // Channel 1 Sweep register       (NR10)
    REG_SOUND1CNT_H = 0x04000062, // Channel 1 Duty/Length/Envelope (NR11, NR12)
    REG_SOUND1CNT_X = 0x04000064, // Channel 1 Frequency/Control    (NR13, NR14)
    // 4000066h     -    -           Not used
    REG_SOUND2CNT_L = 0x04000068, // Channel 2 Duty/Length/Envelope (NR21, NR22)
    // 400006Ah     -    -           Not used
    REG_SOUND2CNT_H = 0x0400006C, // Channel 2 Frequency/Control    (NR23, NR24)
    // 400006Eh     -    -           Not used
    REG_SOUND3CNT_L = 0x04000070, // Channel 3 Stop/Wave RAM select (NR30)
    REG_SOUND3CNT_H = 0x04000072, // Channel 3 Length/Volume        (NR31, NR32)
    REG_SOUND3CNT_X = 0x04000074, // Channel 3 Frequency/Control    (NR33, NR34)
    // 4000076h     -    -           Not used
    REG_SOUND4CNT_L = 0x04000078, // Channel 4 Length/Envelope      (NR41, NR42)
    // 400007Ah     -    -           Not used
    REG_SOUND4CNT_H = 0x0400007C, // Channel 4 Frequency/Control    (NR43, NR44)
    // 400007Eh     -    -           Not used
    REG_SOUNDCNT_L = 0x04000080, // Control Stereo/Volume/Enable   (NR50, NR51)
    REG_SOUNDCNT_H = 0x04000082, // Control Mixing/DMA Control
    REG_SOUNDCNT_X = 0x04000084, // Control Sound on/off           (NR52)
    // 4000086h     -    -           Not used
    REG_SOUNDBIAS = 0x04000088, // Sound PWM Control
    // 400008Ah  ..   -    -         Not used
    REG_WAVE_RAM0 = 0x04000090, // Channel 3 Wave Pattern RAM
    REG_WAVE_RAM1 = 0x04000094, // Channel 3 Wave Pattern RAM
    REG_WAVE_RAM2 = 0x04000098, // Channel 3 Wave Pattern RAM
    REG_WAVE_RAM3 = 0x0400009C, // Channel 3 Wave Pattern RAM
    REG_FIFO_A_L = 0x040000A0,  // Channel A FIFO
    REG_FIFO_A_H = 0x040000A2,  // Channel A FIFO
    REG_FIFO_B_L = 0x040000A4,  // Channel B FIFO
    REG_FIFO_B_H = 0x040000A6,  // Channel B FIFO
    // 40000A8h       -    -         Not used

    /* DMA Transfer Channels */
    REG_DMA0SAD = 0x040000B0,   // DMA 0 Source Address
    REG_DMA0DAD = 0x040000B4,   // DMA 0 Destination Address
    REG_DMA0CNT_L = 0x040000B8, // DMA 0 Word Count
    REG_DMA0CNT_H = 0x040000BA, // DMA 0 Control
    REG_DMA1SAD = 0x040000BC,   // DMA 1 Source Address
    REG_DMA1DAD = 0x040000C0,   // DMA 1 Destination Address
    REG_DMA1CNT_L = 0x040000C4, // DMA 1 Word Count
    REG_DMA1CNT_H = 0x040000C6, // DMA 1 Control
    REG_DMA2SAD = 0x040000C8,   // DMA 2 Source Address
    REG_DMA2DAD = 0x040000CC,   // DMA 2 Destination Address
    REG_DMA2CNT_L = 0x040000D0, // DMA 2 Word Count
    REG_DMA2CNT_H = 0x040000D2, // DMA 2 Control
    REG_DMA3SAD = 0x040000D4,   // DMA 3 Source Address
    REG_DMA3DAD = 0x040000D8,   // DMA 3 Destination Address
    REG_DMA3CNT_L = 0x040000DC, // DMA 3 Word Count
    REG_DMA3CNT_H = 0x040000DE, // DMA 3 Control
    // 40000E0h       -    -         Not used

    /* Timer Registers */
    REG_TM0CNT_L = 0x04000100, // Timer 0 Counter/Reload
    REG_TM0CNT_H = 0x04000102, // Timer 0 Control
    REG_TM1CNT_L = 0x04000104, // Timer 1 Counter/Reload
    REG_TM1CNT_H = 0x04000106, // Timer 1 Control
    REG_TM2CNT_L = 0x04000108, // Timer 2 Counter/Reload
    REG_TM2CNT_H = 0x0400010A, // Timer 2 Control
    REG_TM3CNT_L = 0x0400010C, // Timer 3 Counter/Reload
    REG_TM3CNT_H = 0x0400010E, // Timer 3 Control
    // 4000110h       -    -         Not used

    /* Serial Communication (1) */
    REG_SIODATA32 =
        0x04000120, // SIO Data (Normal-32bit Mode; shared with below)
    REG_SIOMULTI0 = 0x04000120, // SIO Data 0 (Parent)    (Multi-Player Mode)
    REG_SIOMULTI1 = 0x04000122, // SIO Data 1 (1st Child) (Multi-Player Mode)
    REG_SIOMULTI2 = 0x04000124, // SIO Data 2 (2nd Child) (Multi-Player Mode)
    REG_SIOMULTI3 = 0x04000126, // SIO Data 3 (3rd Child) (Multi-Player Mode)
    REG_SIOCNT = 0x04000128,    // SIO Control Register
    REG_SIOMLT_SEND =
        0x0400012A,            // SIO Data (Local of MultiPlayer; shared below)
    REG_SIODATA8 = 0x0400012A, // SIO Data (Normal-8bit and UART Mode)
    // 400012Ch       -    -         Not used

    /* Keypad Input */
    REG_KEYINPUT = 0x04000130, // Key Status
    REG_KEYCNT = 0x04000132,   // Key Interrupt Control

    /* Serial Communication (2) */
    REG_RCNT = 0x04000134, // SIO Mode Select/General Purpose Data
    REG_IR = 0x04000136,   // Ancient - Infrared Register (Prototypes only)
    // 4000138h       -    -         Not used
    REG_JOYCNT = 0x04000140, // SIO JOY Bus Control
    // 4000142h       -    -         Not used
    REG_JOY_RECV = 0x04000150,  // SIO JOY Bus Receive Data
    REG_JOY_TRANS = 0x04000154, // SIO JOY Bus Transmit Data
    REG_JOYSTAT = 0x04000158,   // SIO JOY Bus Receive Status
    // 400015Ah       -    -         Not used

    /* Interrupt, Waitstate, and Power-Down Control */
    REG_IE = 0x04000200,      // Interrupt Enable Register
    REG_IF = 0x04000202,      // Interrupt Request Flags / IRQ Acknowledge
    REG_WAITCNT = 0x04000204, // Game Pak Waitstate Control
    // 4000206h       -    -         Not used
    REG_IME = 0x04000208, // Interrupt Master Enable Register
    // 400020Ah       -    -         Not used
    REG_POSTFLG = 0x04000300, // Undocumented - Post Boot Flag
    REG_HALTCNT = 0x04000301, // Undocumented - Power Down Control
                              // 4000302h       -    -         Not used
    // 4000410h  ?    ?    ?         Undocumented - Purpose Unknown / Bug ???
    // 0FFh 4000411h       -    -         Not used 4000800h  4    R/W  ?
    // Undocumented - Internal Memory Control (R/W) 4000804h       -    - Not
    // used 4xx0800h  4    R/W  ?         Mirrors of 4000800h (repeated each
    // 64K) 4700000h  4    W    (3DS)     Disable ARM7 bootrom overlay (3DS
    // only)
  };

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

  // Internal pixel data
  union {
    uint8_t bytes[4];
    uint32_t full;
  } internalPX[2];

  union {
    uint8_t bytes[4];
    uint32_t full;
  } internalPY[2];

  // lcd data
  LCD lcd;

  uint32_t read_open_bus(uint32_t addr);
  uint32_t read_sram(uint32_t addr);
  void write8_mmio(uint32_t addr, uint8_t data);
  uint8_t read8_mmio(uint32_t addr);
};
