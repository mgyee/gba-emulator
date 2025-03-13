#include "bus.h"
#include "cpu.h"
#include "sdl.h"
#include <cstdio>

#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 240

int main(int argc, char *argv[]) {

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
    return 1;
  }

  // Bus *bus = new Bus();
  std::unique_ptr<Bus> bus = std::make_unique<Bus>();
  CPU *cpu = new CPU(std::move(bus));
  // bus->attach_cpu(std::move(cpu));

  cpu->start(argv[1], "bios.bin");

  sdl_init();

  bool running = true;

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      }
    }
  }

  sdl_quit();

  return 0;
}
