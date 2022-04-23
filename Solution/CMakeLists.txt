cmake_minimum_required(VERSION 3.20)
project(AntiLarsen C)

set(CMAKE_C_STANDARD 11)

add_executable(AntiLarsen main.c iir.c iir.h audio.c audio.h)
target_link_libraries(${PROJECT_NAME} PUBLIC m)