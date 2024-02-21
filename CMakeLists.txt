cmake_minimum_required(VERSION 3.24)
project(gmu)
set(CMAKE_CXX_STANDARD 14)

add_executable(gmu main.cpp)

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)
target_link_libraries(gmu libgme)
target_link_libraries(gmu game-music-emu)