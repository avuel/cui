cmake_minimum_required(VERSION 4.0.0)
project(cui_top)

add_subdirectory(vendor/nuklear_sdl)

set(PLATFORM "SDL" CACHE STRING "" FORCE)
add_subdirectory(vendor/raylib)

add_subdirectory(src)
