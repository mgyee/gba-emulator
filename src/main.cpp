#include "bus.h"
#include "cpu.h"
#include <cstdio>

#define SCREEN_HEIGHT 160
#define SCREEN_WIDTH 240

int main(int argc, char *argv[]) {

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
    return 1;
  }

  Bus *bus = new Bus();
  CPU *cpu = new CPU(*bus);

  return 0;
}
