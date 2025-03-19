#include "ppu.h"
#include "bus.h"

PPU::PPU(Bus &bus) : bus(bus) {
  frame = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
  pitch = 240 * sizeof(uint32_t);
  dots = 0;
  sdl_init();
}

PPU::~PPU() {
  delete[] frame;
  sdl_quit();
}

bool PPU::sdl_init() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    return false;
  }

  window = SDL_CreateWindow("gba-emulator", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  if (!window)
    goto cleanup;

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    goto cleanup;

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, 240, 160);
  if (!texture)
    goto cleanup;

  return true;

cleanup:
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
  return false;
}

void PPU::sdl_quit() {
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void PPU::tick(uint32_t cycles) {
  dots += cycles;
  if (dots >= 1232) {
    dots -= 1232;
    if (lcd.vcount.bits.scanline < SCREEN_HEIGHT) {
      render_scanline(lcd.vcount.bits.scanline);
    }

    lcd.vcount.bits.scanline++;

    if (lcd.vcount.bits.scanline == SCREEN_HEIGHT) {
      lcd.dispstat.bits.vblank = 1;
      SDL_UpdateTexture(texture, NULL, frame, pitch);
      SDL_RenderCopy(renderer, texture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }

    if (lcd.vcount.bits.scanline == 228) {
      lcd.dispstat.bits.vblank = 0;
      lcd.vcount.bits.scanline = 0;
    }
  }
}

void PPU::render_scanline(uint32_t y) {
  uint8_t mode = lcd.dispcnt.bits.bgMode;
  if (mode == 3) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      uint32_t addr = 0x06000000 + (y * SCREEN_WIDTH + x) * 2;
      uint16_t color = bus.read16(addr, CPU::CYCLE_TYPE::FAST);

      uint8_t r = ((color >> 0) & 0x1F) << 3;  // Red (bits 0-4)
      uint8_t g = ((color >> 5) & 0x1F) << 3;  // Green (bits 5-9)
      uint8_t b = ((color >> 10) & 0x1F) << 3; // Blue (bits 10-14)

      uint8_t a = 0xff;
      r |= (r >> 5);
      g |= (g >> 5);
      b |= (b >> 5);

      uint32_t rgb = (a << 24) | (r << 16) | (g << 8) | b; // RGB color

      frame[y * SCREEN_WIDTH + x] = rgb;
    }
  } else if (mode == 4) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      // Calculate the address for the pixel in VRAM
      uint32_t addr = 0x06000000 + (y * SCREEN_WIDTH + x);

      // Read the color index from VRAM at this address
      uint8_t idx = bus.read8(addr, CPU::CYCLE_TYPE::FAST);

      // Read the RGB16 value from the palette memory (assuming 16-bit RGB
      // palette)
      uint16_t bgr = bus.read16(0x05000000 + (idx * 2), CPU::CYCLE_TYPE::FAST);

      // Extract the RGB components from the 16-bit value
      uint8_t r = ((bgr >> 0) & 0x1F) << 3;  // 5 bits for red
      uint8_t g = ((bgr >> 5) & 0x1F) << 3;  // 5 bits for green
      uint8_t b = ((bgr >> 10) & 0x1F) << 3; // 5 bits for blue

      uint8_t a = 0xff;
      r |= (r >> 5);
      g |= (g >> 5);
      b |= (b >> 5);

      uint32_t rgb = (a << 24) | (r << 16) | (g << 8) | b; // RGB color

      frame[y * SCREEN_WIDTH + x] = rgb;
    }
  }
}
