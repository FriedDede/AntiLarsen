
#include <iostream>
#include <jack/jack.h>
#include <jack/types.h>
#include "include/iir.h"
#include "include/peaksFinder.h"
#include "include/activeFilters.h"
#include <cstdlib>
#include <cstring>

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

preFiltersBank *filters;
activeFilters *dsp;
peaksFinder *analyzer;

int process (jack_nframes_t nframes, void *arg){
    float *in, *out;
    in = (float *)jack_port_get_buffer (input_port, nframes);
    out = (float *)jack_port_get_buffer (output_port, nframes);
    analyzer->run(in);

    dsp->setInOutBuffers(in,out, (int) nframes);
    dsp->reset_bank();
    for (auto &f_idx: analyzer->found_howls) {
        if (f_idx != 0){
            dsp->add_filter_to_bank(f_idx, filters->filters);
        }
    }
    dsp->applyFilters();
    memcpy (out, dsp->getBufOut(),sizeof (jack_default_audio_sample_t) * nframes);

    return 0;
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown (void *arg)
{
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
    analyzer_settings[0]= true; // enable pnpr
    analyzer_settings[1]= true; // enable phpr
    analyzer_settings[2]= false; // enable imsd
    float f_sampling = 44100;
    float gb = -10;
    float q_factor = 30;
    float f_min  = 20;
    /* Plugin init */
    filters = new preFiltersBank(f_sampling,gb,q_factor,f_min);
    dsp = new activeFilters;
    analyzer = new peaksFinder(analyzer_settings);
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

    /* tell the JACK server to call `process()' whenever
       there is work to be done.
    */

    jack_set_process_callback (client, process, 0);

    /* tell the JACK server to call `jack_shutdown()' if
       it ever shuts down, either entirely, or if it
       just decides to stop calling us.
    */

    jack_on_shutdown (client, jack_shutdown, 0);

    /* display the current sample rate.
     */

    printf ("engine sample rate: %" PRIu32 "\n",
            jack_get_sample_rate (client));

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
     * process() callback will start running now. */

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

    ports = jack_get_ports (client, nullptr, nullptr,
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

    free (ports);

    /* keep running until stopped by the user */

    while(true){};

    /* this is never reached but if the program
       had some other way to exit besides being killed,
       they would be important to call.
    */

    jack_client_close (client);
    exit (0);
}






