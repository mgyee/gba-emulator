#pragma once
#include <cstdint>

struct LCD {
  /*
   * Unions make each part (full, bytes, bits) occupy same space in memory,
   * so setting full or bytes when R/W will fill out bits section for us,
   * making accessing specific bits easy
   */
  union {
    struct {
      uint8_t bgMode : 3;   // BG Mode                    (0-5=Video Mode 0-5,
                            // 6-7=Prohibited)
      bool cbgMode : 1;     // Reserved / CGB Mode        (0=GBA, 1=CGB; can be
                            // set only by BIOS opcodes)
      bool frameSelect : 1; // Display Frame Select       (0-1=Frame 0-1) (for
                            // BG Modes 4,5 only)
      bool hblackInterval : 1; // H-Blank Interval Free      (1=Allow access
                               // to OAM during H-Blank)
      bool objCharMap : 1;     // OBJ Character VRAM Mapping (0=Two dimensional,
                               // 1=One dimensional)
      bool forcedBlank : 1; // Forced Blank               (1=Allow FAST access
                            // to VRAM,Palette,OAM)
      bool bg0 : 1;         // Screen Display BG0         (0=Off, 1=On)
      bool bg1 : 1;         // Screen Display BG1         (0=Off, 1=On)
      bool bg2 : 1;         // Screen Display BG2         (0=Off, 1=On)
      bool bg3 : 1;         // Screen Display BG3         (0=Off, 1=On)
      bool obj : 1;         // Screen Display OBJ         (0=Off, 1=On)
      bool win0 : 1;        // Window 0 Display Flag      (0=Off, 1=On)
      bool win1 : 1;        // Window 1 Display Flag      (0=Off, 1=On)
      bool objWin : 1;      // OBJ Window Display Flag    (0=Off, 1=On)
    } bits;
    uint8_t bytes[2]; // Break full halfword into R/W-able bytes
    uint16_t full;
  } dispcnt;

  union {
    uint8_t bytes[2];
    uint16_t full;
  } greenswp;

  union {
    struct {
      bool vblank : 1;    // V-Blank flag   (Read only) (1=VBlank) (set in line
                          // 160..226; not 227)
      bool hblank : 1;    // H-Blank flag   (Read only) (1=HBlank) (toggled in
                          // all lines, 0..227)
      bool vcounter : 1;  // V-Counter flag (Read only) (1=Match)  (set in
                          // selected line)     (R)
      bool vblankIRQ : 1; // V-Blank IRQ Enable         (1=Enable) (R/W)
      bool hblankIRQ : 1; // H-Blank IRQ Enable         (1=Enable) (R/W)
      bool vcounterIRQ : 1; // V-Counter IRQ Enable       (1=Enable) (R/W)
      bool : 1; // Not used (0) / DSi: LCD Initialization Ready (0=Busy,
                // 1=Ready)   (R)
      bool : 1; // Not used (0) / NDS: MSB of V-Vcount Setting (LYC.Bit8)
                // (0..262)(R/W)
      uint8_t vcountSetting : 8; // V-Count Setting (LYC)      (0..227) (R/W)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } dispstat;

  union {
    struct {
      uint8_t scanline : 8; // Current Scanline (LY)      (0..227) (R)
      bool : 1;    // Not used (0) / NDS: MSB of Current Scanline (LY.Bit8)
                   // (0..262)   (R)
      uint8_t : 8; // Not Used (0)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } vcount;

  union {
    struct {
      uint8_t bgPriority : 2; // BG Priority           (0-3, 0=Highest)
      uint8_t
          charBase : 2; // Character Base Block  (0-3, in units of 16 KBytes)
                        // (=BG Tile Data)
      uint8_t : 2;      // Not used (must be zero) (except in NDS mode: MSBs of
                        // char base)
      bool mosaic : 1;  // Mosaic                (0=Disable, 1=Enable)
      bool palette : 1; // Colors/Palettes       (0=16/16, 1=256/1)
      uint8_t screenBase : 5; // Screen Base Block     (0-31, in units of 2
                              // KBytes) (=BG Map Data)
      bool wrap : 1; // BG0/BG1: Not used (except in NDS mode: Ext Palette
                     // Slot for BG0/BG1) BG2/BG3: Display Area Overflow
                     // (0=Transparent, 1=Wraparound)
      uint8_t screenSize : 2; // Screen Size (0-3)
                              // Value  Text Mode      Rotation/Scaling Mode
                              // 0      256x256 (2K)   128x128   (256 bytes)
                              // 1      512x256 (4K)   256x256   (1K)
                              // 2      256x512 (4K)   512x512   (4K)
                              // 3      512x512 (8K)   1024x1024 (16K)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgcnt[4]; // BG0-BG3 are structured the same, so we can make this an array
              // to save code other structs below will also follow this
              // pattern

  union {
    struct {
      uint16_t offset : 9; // Offset (0-511)
      uint8_t : 7;         // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bghofs[4];

  union {
    struct {
      uint16_t offset : 9; // Offset (0-511)
      uint8_t : 7;         // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgvofs[4];

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint8_t integer : 7;    // Integer portion    (7 bits)
      bool sign : 1;          // Sign               (1 bit)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgpa[2]; // BG2-BG3, same as the next few below

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint8_t integer : 7;    // Integer portion    (7 bits)
      bool sign : 1;          // Sign               (1 bit)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgpb[2];

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint8_t integer : 7;    // Integer portion    (7 bits)
      bool sign : 1;          // Sign               (1 bit)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgpc[2];

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint8_t integer : 7;    // Integer portion    (7 bits)
      bool sign : 1;          // Sign               (1 bit)
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bgpd[2];

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint32_t integer : 19;  // Integer portion    (19 bits)
      bool sign : 1;          // Sign               (1 bit)
      uint8_t : 4;            // Not used
    } bits;
    uint8_t bytes[4];
    uint16_t halfwords[2];
    uint32_t full;
  } bgx[2]; // BG2-BG3

  union {
    struct {
      uint8_t fractional : 8; // Fractional portion (8 bits)
      uint32_t integer : 19;  // Integer portion    (19 bits)
      bool sign : 1;          // Sign               (1 bit)
      uint8_t : 4;            // Not used
    } bits;
    uint8_t bytes[4];
    uint16_t halfwords[2];
    uint32_t full;
  } bgy[2]; // BG2-BG3

  union {
    struct {
      uint8_t maxH : 8; // X2, Rightmost coordinate of window, plus 1
      uint8_t minH : 8; // X1, Leftmost coordinate of window
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } winh[2]; // WIN0-WIN1

  union {
    struct {
      uint8_t maxV : 8; // Y2, Bottom-most coordinate of window, plus 1
      uint8_t minV : 8; // Y1, Top-most coordinate of window
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } winv[2]; // WIN0-WIN1

  union {
    struct {
      uint8_t win0BG : 4;   // Window 0 BG0-BG3 Enable Bits     (0=No Display,
                            // 1=Display)
      bool win0OBJ : 1;     // Window 0 OBJ Enable Bit          (0=No Display,
                            // 1=Display)
      bool win0Effects : 1; // Window 0 Color Special Effect    (0=Disable,
                            // 1=Enable)
      uint8_t : 2;          // Not used
      uint8_t win1BG : 4;   // Window 1 BG0-BG3 Enable Bits     (0=No Display,
                            // 1=Display)
      bool win1OBJ : 1;     // Window 1 OBJ Enable Bit          (0=No Display,
                            // 1=Display)
      bool win1Effects : 1; // Window 1 Color Special Effect    (0=Disable,
                            // 1=Enable)
      uint8_t : 2;          // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } winin;

  union {
    struct {
      uint8_t outBG : 4;   // Outside BG0-BG3 Enable Bits      (0=No Display,
                           // 1=Display)
      bool outOBJ : 1;     // Outside OBJ Enable Bit           (0=No Display,
                           // 1=Display)
      bool outEffects : 1; // Outside Color Special Effect     (0=Disable,
                           // 1=Enable)
      uint8_t : 2;         // Not used
      uint8_t objBG : 4;   // OBJ Window BG0-BG3 Enable Bits   (0=No Display,
                           // 1=Display)
      bool objOBJ : 1;     // OBJ Window OBJ Enable Bit        (0=No Display,
                           // 1=Display)
      bool objEffects : 1; // OBJ Window Color Special Effect  (0=Disable,
                           // 1=Enable)
      uint8_t : 2;         // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } winout;

  union {
    struct {
      uint8_t bgHSize : 4;  // BG Mosaic H-Size  (minus 1)
      uint8_t bgVSize : 4;  // BG Mosaic V-Size  (minus 1)
      uint8_t objHSize : 4; // OBJ Mosaic H-Size (minus 1)
      uint8_t objVSize : 4; // OBJ Mosaic V-Size (minus 1)
      uint16_t : 16;        // Not used
    } bits;
    uint8_t bytes[4];
    uint8_t halfwords[2];
    uint32_t full;
  } mosaic;

  union {
    struct {
      bool bg01 : 1; // BG0 1st Target Pixel (Background 0)
      bool bg11 : 1; // BG1 1st Target Pixel (Background 1)
      bool bg21 : 1; // BG2 1st Target Pixel (Background 2)
      bool bg31 : 1; // BG3 1st Target Pixel (Background 3)
      bool obj1 : 1; // OBJ 1st Target Pixel (Top-most OBJ pixel)
      bool bd1 : 1;  // BD  1st Target Pixel (Backdrop)
      uint8_t
          effect : 2; // Color Special Effect (0-3, see below)
                      // 0 = None                (Special effects disabled)
                      // 1 = Alpha Blending      (1st+2nd Target mixed)
                      // 2 = Brightness Increase (1st Target becomes whiter)
                      // 3 = Brightness Decrease (1st Target becomes blacker)
      bool bg02 : 1;  // BG0 2nd Target Pixel (Background 0)
      bool bg12 : 1;  // BG1 2nd Target Pixel (Background 1)
      bool bg22 : 1;  // BG2 2nd Target Pixel (Background 2)
      bool bg32 : 1;  // BG3 2nd Target Pixel (Background 3)
      bool obj2 : 1;  // OBJ 2nd Target Pixel (Top-most OBJ pixel)
      bool bd2 : 1;   // BD  2nd Target Pixel (Backdrop)
      uint8_t : 2;    // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bldcnt;

  union {
    struct {
      uint8_t eva : 5; // EVA Coefficient (1st Target) (0..16 =
                       // 0/16..16/16, 17..31=16/16)
      uint8_t : 3;     // Not used
      uint8_t evb : 5; // EVB Coefficient (2nd Target) (0..16 =
                       // 0/16..16/16, 17..31=16/16)
      uint8_t : 3;     // Not used
    } bits;
    uint8_t bytes[2];
    uint16_t full;
  } bldalpha;

  union {
    struct {
      uint8_t evy : 5; // EVY Coefficient (Brightness) (0..16 =
                       // 0/16..16/16, 17..31=16/16)
      uint32_t : 26;   // Not used
    } bits;
    uint8_t bytes[4];
    uint16_t halfwords[2];
    uint32_t full;
  } bldy;

  // The below is not in the GBATEK but will be helpful for oam rendering
  union {
    struct {
      struct {
        uint16_t coordY : 8;
        uint16_t affine : 1;
        uint16_t virtSize : 1;
        uint16_t mode : 2;
        uint16_t mosaic : 1;
        uint16_t color256 : 1;
        uint16_t sizeHigh : 2;
      } one;
      union {
        struct {
          uint16_t coordX : 9;
          uint16_t : 3;
          uint16_t hflip : 1;
          uint16_t vflip : 1;
          uint16_t sizeLow : 2;
        } two;
        struct {
          uint16_t : 9; // coordX
          uint16_t affineDataIdx : 5;
          uint16_t : 2; // sizeLow
        } affineIdx;
      };
      struct {
        uint16_t tileIdx : 10;
        uint16_t priority : 2;
        uint16_t paletteNum : 4;
      } three;
    } full;
    uint16_t halfwords[3];
  } oamdata;
};
