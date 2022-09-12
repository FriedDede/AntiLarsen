cmake_minimum_required(VERSION 3.16)
project(Jack_AntiLarsen)

set(CMAKE_CXX_STANDARD 20)

add_executable(Jack_AntiLarsen
        main.cpp
        src/fastmath.cpp
        src/Analyzer.cpp
        src/DSP.cpp
        include/Analyzer.h
        include/const.h
        include/fastmath.h
        include/localtools.h
        include/DSP.h
        )
add_executable(TEST
        test.cpp
        src/fastmath.cpp
        src/Analyzer.cpp
        src/DSP.cpp
        include/Analyzer.h
        include/const.h
        include/fastmath.h
        include/localtools.h
        include/DSP.h
        )
add_executable(RTA
        rta.cpp
        src/fastmath.cpp
        src/Analyzer.cpp
        src/DSP.cpp
        include/Analyzer.h
        include/const.h
        include/fastmath.h
        include/localtools.h
        include/DSP.h
        imgui/imgui.h
        imgui/
        src/client.cpp src/client.h)

target_link_libraries(Jack_AntiLarsen
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
        )
target_link_libraries(TEST
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
        )
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(RTA
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
        ${SDL2_LIBRARIES}
        )
