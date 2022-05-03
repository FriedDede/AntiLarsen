cmake_minimum_required(VERSION 3.20)
project(Jack_AntiLarsen)

set(CMAKE_CXX_STANDARD 20)

add_executable(Jack_AntiLarsen /
        main.cpp /
        src/iir.cpp /
        include/iir.h /
        src/peaksFinder.cpp /
        include/peaksFinder.h /
        include/const.h src/fastmath.cpp include/fastmath.h)
