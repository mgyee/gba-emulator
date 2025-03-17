#pragma once
#include "mmio.h"
#include <SDL2/SDL.h>
#include <cstdint>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160

class Bus;

class PPU {
public:
  PPU(Bus &bus);
  ~PPU();

  void tick();

  bool sdl_init();

  void sdl_quit();

  LCD lcd;

private:
  Bus &bus;

  uint32_t dots;

  uint32_t *frame;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t pitch;

  void render_scanline(uint32_t y);
};
