cmake_minimum_required(VERSION 3.28)

project(gba_emulator)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(gba_emulator src/main.cpp src/cpu.cpp)
