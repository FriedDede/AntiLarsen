//
// Created by andre on 8/27/22.
//
#include "imgui/imgui.h"
#include "imgui/opengl3/imgui_impl_sdl.h"
#include "imgui/opengl3/imgui_impl_opengl3.h"

#include "include/Analyzer.h"
#include "include/DSP.h"
#include "include/fastmath.h"
#include "include/client.h"
#include "include/rta.h"

#include <iostream>
#include <jack/jack.h>
#include <jack/types.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <vector>
#include <thread>
#include <SDL.h>

#define IMGUI_IMPL_OPENGL_LOADER_GL3W 1
#define refresh_interval 45
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include "imgui/libs/gl3w/GL/gl3w.h" // Initialize with gl3wInit()
#endif

std::vector <client *> clients;

void GUI() {
    float data[FFTOUT_BUF_LENGTH];
    const char **ports;
    int in_ports = 0;
    int out_ports;
    std::vector<std::vector<u_int8_t>> input_state;

    float sub_freq = 0.0f;
    bool sub_gradient = false;
    bool sub_endfire = false;
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        exit(-1);
    }
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                                      SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("RTA", SDL_WINDOWPOS_CENTERED, \
    SDL_WINDOWPOS_CENTERED, 1280, 720,window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#else
    bool err = false;
#endif
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        exit(1);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsClassic();
    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
    SDL_Event event;

    bool done = false;
    bool show_rta = false;
    bool show_in = false;
    bool show_out = false;
    bool show_sub = false;
    do{
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE\
            && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame(window);
            ImGui::NewFrame();
        }
        // MAIN MENU
        ImGui::BeginMainMenuBar();
        if(ImGui::MenuItem("add RTA", nullptr, &show_rta)){
            rta *new_rta = new rta("rta");
            clients.push_back(new_rta);
        };
        ImGui::MenuItem("SUB placer", nullptr,&show_sub);
        ImGui::MenuItem("IN list", nullptr,&show_in);
        ImGui::MenuItem("OUT list", nullptr, &show_out);
        ImGui::EndMainMenuBar();
        //RTA Window
        for (auto &c: clients) {
            if(c->type == mag_rta && c->enabled){
                auto c_rta = (rta *)c;
                for (int i = 0; i < FFTOUT_BUF_LENGTH*c_rta->getFftLength(); ++i) {
                    data[i] = 10.0f*log10f_fast(std::abs(c_rta->getFtOut()[i]))+60.0f;
                }
                ImGui::Begin(jack_get_client_name(c->getJackClient()), &c->enabled);
                ImGui::PushItemWidth(-1);
                ImGui::PlotHistogram("RTA",data,FFTOUT_BUF_LENGTH, \
                0, NULL, 0.0f, 60.0f, ImGui::GetWindowSize());
                ImGui::End();
            }
        }
        if (show_in){
            int n_ports = 0;
            ImGui::Begin("Capture", &show_in);
            ports = jack_get_ports (clients[0]->getJackClient(), nullptr,nullptr,
                                    JackPortIsPhysical|JackPortIsOutput);
            for (n_ports = 0; ports[n_ports] != nullptr; ++n_ports);
            if (in_ports != n_ports){
                input_state.resize(n_ports);
                for (auto &v : input_state) v.resize(clients.size());
                in_ports = n_ports;
            }
            ImGui::Columns((int) clients.size()+1, nullptr, true);
            ImGui::Text("Ports:");
            ImGui::NextColumn();
            for(auto &c : clients){
                ImGui::Text("%s", jack_get_client_name(c->getJackClient()));
                ImGui::NextColumn();
            }
            for (int i = 0; i < n_ports ; ++i) {
                ImGui::Text("%s",ports[i]);
                ImGui::NextColumn();
                int j = 0;
                for(auto &c : clients) {
                    std::string label;
                    // TODO: Che cazzo succede qua ???
                    label = "##" + std::to_string(i) + std::to_string(j);
                    if(ImGui::Checkbox(label.c_str(), (bool *)(&input_state[i][j]))){
                        std::cout << "checkbox click" <<std::endl;
                        if (input_state[i][j]) {
                            jack_connect(c->getJackClient(), ports[i],\
                            jack_port_name(c->input_port));
                            std::cout << "connect port: " << i << " to client: " << j <<std::endl;
                        }
                        else if (!input_state[i][j]) {
                            jack_disconnect(c->getJackClient(), ports[i],\
                            jack_port_name(c->input_port));
                            std::cout << "disconnect port: " << i << " from client: " << j <<std::endl;
                        }
                    }
                    ++j;
                    ImGui::NextColumn();
                }
            }
            ImGui::End();
        }
        if (show_out){
            ImGui::Begin("Output", &show_out);
            ports = jack_get_ports (clients[0]->getJackClient(), nullptr,\
            nullptr,JackPortIsPhysical|JackPortIsInput);
            const char ** cursor = ports;
            while(*cursor != nullptr){
                ImGui::SmallButton(cursor[0]);
                cursor++;
            }
            ImGui::End();

        }
        if (show_sub){

            ImGui::Begin("SubWoofer Placer", &show_sub);
            ImGui::InputFloat("Frequency [Hz]: ",&sub_freq, 0.1f, 1.0f);
            ImGui::Checkbox("Endfire", &sub_endfire);
            if(sub_endfire)sub_gradient= false;
            ImGui::Checkbox("Gradient", &sub_gradient);
            if (sub_gradient){
                sub_endfire = false;
                ImGui::Text("Back row:");
                ImGui::Text("Delay: + %f ms", (1.0f/sub_freq)/4*1000);
                ImGui::Text("Phase: 180°");
                ImGui::Text("Front row:");
                ImGui::Text("Distance: %f m", (1.0f/sub_freq)/4*331.45f);
            }
            if (sub_endfire){
                ImGui::Text("Back row:");
                ImGui::Text("Flat");
                ImGui::Text("Front row:");
                ImGui::Text("Distance: %f m", (1.0f/sub_freq)/4*331.45f);
                ImGui::Text("Delay: + %f ms", (1.0f/sub_freq)/4*1000);
            }
            ImGui::End();
        }
        // RENDER COMMIT
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }while(!done);


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
};

int main (int argc, char *argv[])
{
    auto *test = new client("test");
    auto *rta_test = new  rta("rta");
    clients.push_back(rta_test);
    clients.push_back(test);

    std::thread gui(GUI);
    gui.detach();

    sleep(-1);
    for (auto &c: clients) {
        jack_deactivate(c->getJackClient());
    }
    exit (0);
}