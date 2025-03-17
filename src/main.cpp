#include "bus.h"
#include <cstdio>

#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 240

int main(int argc, char *argv[]) {

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
    return 1;
  }

  CPU *cpu = new CPU();

  Bus *bus = new Bus(*cpu);

  PPU *ppu = new PPU(*bus);

  bus->attach_ppu(ppu);

  cpu->set_bus(bus);

  // ppu->sdl_init();

  cpu->start(argv[1], "../bios.bin");

  // while (running) {
  //   SDL_Event event;
  //   while (SDL_PollEvent(&event)) {
  //     switch (event.type) {
  //     case SDL_QUIT:
  //       running = false;
  //       break;
  //     }
  //   }
  // }

  // ppu->sdl_quit();
  delete cpu;

  return 0;
}
