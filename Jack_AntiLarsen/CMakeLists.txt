cmake_minimum_required(VERSION 3.16)
project(Jack_AntiLarsen)

set(CMAKE_CXX_STANDARD 20)

add_executable(Jack_AntiLarsen
        main.cpp
        src/iir.cpp
        src/fastmath.cpp
        src/peaksFinder.cpp
        src/activeFilters.cpp
        include/peaksFinder.h
        include/const.h
        include/iir.h
        include/fastmath.h
        include/localtools.h
        include/activeFilters.h
        )

target_link_libraries(Jack_AntiLarsen
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3
        )