//
// Created by andre on 8/27/22.
//

#include "imgui/imgui.h"
#include "imgui/opengl3/imgui_impl_sdl.h"
#include "imgui/opengl3/imgui_impl_opengl3.h"

#include "include/Analyzer.h"
#include "include/DSP.h"
#include "include/fastmath.h"

#include <iostream>
#include <jack/jack.h>
#include <jack/types.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <vector>
#include <thread>
#include <SDL.h>

#define IMGUI_IMPL_OPENGL_LOADER_GL3W 1
#define refresh_interval 45
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include "imgui/libs/gl3w/GL/gl3w.h" // Initialize with gl3wInit()
#endif

jack_port_t *input_port;
jack_port_t *output_port;

Analyzer *analyzer;

int process(jack_nframes_t nframes, void *arg){
    float *in;
    in = (float *)jack_port_get_buffer (input_port, nframes);
    analyzer->analyzeBuffer(in);
    return 0;
};

void jack_shutdown (void *arg){
    delete analyzer;
    exit (1);
}

void GUI(std::vector <jack_client_t *> clients) {
    float data[FFTOUT_BUF_LENGTH];
    const char **ports;
    int in_ports = 0;
    int out_ports;
    std::vector<std::vector<u_int8_t>> input_new_state;
    std::vector<std::vector<u_int8_t>> input_state;
    for(auto &v : input_new_state){
        for(auto &i: v){
            i = 0;
        }
    }
    for(auto &v : input_state){
        for(auto &i:v){
            i = 0;
        }
    }

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
    SDL_Window *window = SDL_CreateWindow("RTA", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
                                          window_flags);
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
            ImGui::Begin("Main Menù");
            ImGui::Checkbox("RTA", &show_rta);
            ImGui::Checkbox("SUB placer", &show_sub);
            ImGui::Checkbox("IN list", &show_in);
            ImGui::Checkbox("OUT list", &show_out);
            ImGui::End();
        }
        //RTA Window
        if(show_rta){
            for (int i = 0; i < FFTOUT_BUF_LENGTH; ++i) {
                data[i] = 10.0f*log10f_fast(analyzer->getOutBuffer()[i])+60.0f;
            }
            ImGui::Begin("RTA", &show_rta);
            ImGui::PushItemWidth(-1);
            ImGui::PlotHistogram("RTA",data,FFTOUT_BUF_LENGTH, \
            0, NULL, 0.0f, 60.0f, ImGui::GetWindowSize());
            ImGui::End();
        }
        if (show_in){
            int n_ports = 0;
            ports = jack_get_ports (clients[0], nullptr,nullptr,
                                    JackPortIsOutput);
            for (n_ports = 0; ports[n_ports] != nullptr; ++n_ports);
            if (in_ports != n_ports){
                input_new_state.resize(n_ports);
                input_state.resize(n_ports);
                for (auto &v : input_new_state) v.resize(clients.size());
                for (auto &v : input_state) v.resize(clients.size());
                in_ports = n_ports;
            }
            ImGui::Begin("Capture", &show_in);
            ImGui::Columns((int) clients.size()+1, nullptr, true);
            ImGui::Text("Ports:");
            ImGui::NextColumn();
            for(auto &c : clients){
                ImGui::Text("%s", jack_get_client_name(c));
                ImGui::NextColumn();
            }
            for (int i = 0; i < n_ports ; ++i) {
                ImGui::Text("%s",ports[i]);
                ImGui::NextColumn();
                int j = 0;
                for(auto &c : clients) {
                    ImGui::Checkbox("", reinterpret_cast<bool *>(& input_new_state[i][j]));
                    ImGui::NextColumn();
                    if (input_new_state[i][j] && !input_state[i][j]) {
                        jack_connect(c, ports[i], jack_port_name(input_port));
                        input_state[i][j] = input_new_state[i][j];
                    }
                    if (!input_new_state[i][j] && input_state[i][j]) {
                        jack_disconnect(c, ports[i], jack_port_name(input_port));
                        input_state[i][j] = input_new_state[i][j];
                    }
                    ++j;
                }
            }
            ImGui::End();
        }
        if (show_out){
            ImGui::Begin("Output", &show_out);
            ports = jack_get_ports (clients[0], nullptr,nullptr,
                                    JackPortIsPhysical|JackPortIsInput);
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
    std::vector <jack_client_t *> clients;

    const char **ports;
    const char *client_name = "RTA";
    const char *server_name = nullptr;
    jack_client_t *client;
    jack_options_t options = JackNullOption;
    jack_status_t status;
    bool analyzer_settings[3];

    /* Default settings */
    analyzer_settings[0]= true; // enable phpr
    analyzer_settings[1]= true; // enable pnpr
    analyzer_settings[2]= true; // enable imsd


    /* open a client connection to the JACK server */
    client = jack_client_open (client_name, options, &status, server_name);
    if (client == nullptr) {
        fprintf (stderr, "jack_client_open() failed, "
                         "status = 0x%2.0x\n", status);
        if (status & JackServerFailed) {
            fprintf (stderr, "Unable to connect to JACK server\n");
        }
        exit (1);
    }
    if (status & JackServerStarted) {
        fprintf (stderr, "JACK server started\n");
    }
    if (status & JackNameNotUnique) {
        client_name = jack_get_client_name(client);
        fprintf (stderr, "unique name `%s' assigned\n", client_name);
    }

    /* Plugin init */
    std::cout << "---------------- " << client_name << " -----------------" << std::endl;

    analyzer = new Analyzer(analyzer_settings);
    if (analyzer == nullptr){
        std::cout << "Unable to initialize analyzer, quitting."<< std::endl;
        exit (EXIT_FAILURE);
    }
    else {
        std::cout << "-------- Analyzer settings:"<< std::endl;
        if (analyzer->isRunPhpr()) {
            std::cout << "   Power to Harmonics power ratio: " << std::endl <<
                      "   threshold: " << analyzer->getPhprThreshold() << std::endl;
        }
        if (analyzer->isRunPnpr()){
            std::cout << "   Power to Neighbours power ratio: "<< std::endl <<
                      "   threshold: " << analyzer->getPnprThreshold() << std::endl;
        }
        if (analyzer->isRunImsd()){
            std::cout << "   Inter-frame Magnitude Slope Deviation: "<< std::endl <<
                      "   threshold: " << analyzer->getImsdThreshold() << std::endl;
        }
        std::cout <<"-------- Found Larsen (Hz):" << std::endl;
    }

    /* tell the JACK server to call `process()' wheneverS
     * there is work to be done.
     * */
    jack_set_process_callback (client, process, 0);
    /* tell the JACK server to call `jack_shutdown()' if
     * it ever shuts down, either entirely, or if it
     * just decides to stop calling us.
     * */
    jack_on_shutdown (client, jack_shutdown, 0);

    /* create two ports */
    input_port = jack_port_register (client, "in",
                                     JACK_DEFAULT_AUDIO_TYPE,
                                     JackPortIsInput, 0);
    output_port = jack_port_register (client, "out",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
    if ((input_port == nullptr) || (output_port == nullptr)) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
    /* Tell the JACK server that we are ready to roll.  Our
     * process() callback will start running now.
     * */
    if (jack_activate (client) == EXIT_SUCCESS) {
        clients.push_back(client);
        clients.shrink_to_fit();
    }
    else{
        fprintf (stderr, "cannot activate client");
        exit (1);
    }

    std::thread gui(GUI,clients);
    gui.detach();

    sleep(-1);
    jack_client_close (client);
    exit (0);
}