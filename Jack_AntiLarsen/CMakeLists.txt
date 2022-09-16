cmake_minimum_required(VERSION 3.16)
project(Jack_AntiLarsen)

set(CMAKE_CXX_STANDARD 20)


find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

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

target_link_libraries(Jack_AntiLarsen
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
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
target_link_libraries(TEST
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
        )

## RTA Recipe
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
        src/client.cpp
        include/client.h


        imgui/
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/opengl3/imgui_impl_sdl.cpp
        imgui/opengl3/imgui_impl_opengl3.cpp
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/libs/gl3w/GL/gl3w.c
        ##imgui/opengl3/imgui.o
        ##imgui/opengl3/imgui_draw.o
        ##imgui/opengl3/imgui_impl_opengl3.o
        ##imgui/opengl3/imgui_impl_sdl.o
        ##imgui/opengl3/imgui_widgets.o
        )
include_directories(RTA
        imgui/libs/gl3w/GL
        imgui)
target_link_libraries(RTA
        /usr/lib64/pipewire-0.3/jack/libjack.so.0
        fftw3f
        ${SDL2_LIBRARIES}
        GL
        )
add_compile_options(RTA
        -Iimgui/
        -Iimgui/opengl3/
        -Isrc/
        -Iinclude/
        -Wformat
        -Iimgui/libs/gl3w
        -DIMGUI_IMPL_OPENGL_LOADER_GL3W
        `sdl2-config --cflags`
        `sdl2-config --libs`
        -ldl
        )

