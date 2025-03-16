cmake_minimum_required(VERSION 3.28)

project(gba_emulator)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

include_directories(include)

add_executable(gba_emulator src/main.cpp src/cpu.cpp src/bus.cpp src/ppu.cpp src/arm.cpp)

target_link_libraries(gba_emulator SDL2::SDL2)
