
#include <iostream>
#include <jack/jack.h>
#include <jack/types.h>
#include "include/Analyzer.h"
#include "include/DSP.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

DSP *dsp;
Analyzer *analyzer;

/**
 * JACK process_callback is run once per JACK server cycle
 */
int process (jack_nframes_t nframes, void *arg){
    float *in, *out;
    bool larsen = false;
    in = (float *)jack_port_get_buffer (input_port, nframes);
    out = (float *)jack_port_get_buffer (output_port, nframes);
    analyzer->analyzeBuffer(in);

    dsp->setInOutBuffers(in,out, (int) nframes);
    for (auto f_idx: analyzer->found_howls) {
        if (f_idx != 0){
            dsp->add_filter_to_bank(f_idx);
            larsen = true;
            // debug only
            std::cout << f_idx*FSTEP << std::endl;
        }
    }
    if (larsen) dsp->applyFilters();
    else memcpy (out, in,sizeof (jack_default_audio_sample_t) * nframes);

    dsp->reset_bank();
    return 0;
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg){
    delete dsp;
    delete analyzer;
    exit (1);
}

int main (int argc, char *argv[])
{
    const char **ports;
    const char *client_name = "FDB-Killer";
    const char *server_name = nullptr;
    jack_options_t options = JackNullOption;
    jack_status_t status;
    bool analyzer_settings[3];

    /* Default settings */
    analyzer_settings[0]= true; // enable phpr
    analyzer_settings[1]= true; // enable pnpr
    analyzer_settings[2]= true; // enable imsd

    float gb = 0.01f;
    float q_factor = 35;
    float f_min  = 0;

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

    dsp = new DSP;
    if (dsp == nullptr){
        std::cout << "Unable to initialize dsp, quitting."<< std::endl;
        exit(EXIT_FAILURE);
    }
    else{
        std::cout << "--------- Filters settings:" << std::endl;
        std::cout << "  Engine sample rate: " << (float )jack_get_sample_rate(client) << " Hz" << std::endl;
        std::cout << "  Gain at cut-off:    " << gb << " dB" << std::endl;
        std::cout << "  Q factor:           " << q_factor <<std::endl;
        std::cout << "  Min Frequency:      " << f_min << " Hz" << std::endl;
    }
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
    input_port = jack_port_register (client, "input",
                                     JACK_DEFAULT_AUDIO_TYPE,
                                     JackPortIsInput, 0);
    output_port = jack_port_register (client, "output",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
    if ((input_port == nullptr) || (output_port == nullptr)) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
    /* Tell the JACK server that we are ready to roll.  Our
     * process() callback will start running now.
     * */
    if (jack_activate (client)) {
        fprintf (stderr, "cannot activate client");
        exit (1);
    }

    /* Connect the ports.  You can't do this before the client is
     * activated, because we can't make connections to clients
     * that aren't running.  Note the confusing (but necessary)
     * orientation of the driver backend ports: playback ports are
     * "input" to the backend, and capture ports are "output" from
     * it.
     */
    /*
    ports = jack_get_ports (client, nullptr,nullptr,
                            JackPortIsPhysical|JackPortIsOutput);
    if (ports == nullptr) {
        fprintf(stderr, "no physical capture ports\n");
        exit (1);
    }
    if (jack_connect (client, ports[0], jack_port_name (input_port))) {
        fprintf (stderr, "cannot connect input ports\n");
    }
    free (ports);

    ports = jack_get_ports (client, nullptr, nullptr,
                            JackPortIsPhysical|JackPortIsInput);
    if (ports == nullptr) {
        fprintf(stderr, "no physical playback ports\n");
        exit (1);
    }
    if (jack_connect (client, jack_port_name (output_port), ports[0])) {
        fprintf (stderr, "cannot connect output ports\n");
    }
    */
    free (ports);

    /* keep running until stopped by the user */
    sleep(-1);
    /* this is never reached but if the program
     * had some other way to exit besides being killed,
     * they would be important to call.
     * */
    jack_client_close (client);
    exit (0);
}






