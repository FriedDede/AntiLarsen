//
// Created by andre on 9/12/22.
//

#include "client.h"
#include <jack/jack.h>
#include <jack/types.h>
#include <iostream>

int jack_callbacks::process(jack_nframes_t nframes, void *clientpointer) {
    auto *cp = reinterpret_cast<class client *>(clientpointer);
    cp->process(nframes,0);
    return 0;
}

void jack_callbacks::shutdown(void *clientpointer) {
    auto *cp = reinterpret_cast<class client *>(clientpointer);
    cp->shutdown(0);
}

client::client(char *client_name) {
    this->jack_client = jack_client_open (client_name, this->jack_options,\
    &this->jack_status, nullptr);
    if (this->jack_client == nullptr) {
        fprintf (stderr, "jack_client_open() failed, "
                         "status = 0x%2.0x\n", this->jack_status);
        if (this->jack_status & JackServerFailed) {
            fprintf (stderr, "Unable to connect to JACK server\n");
        }
        exit (1);
    }
    if (this->jack_status & JackServerStarted) {
        fprintf (stderr, "JACK server started\n");
    }
    if (this->jack_status & JackNameNotUnique) {
        client_name = jack_get_client_name(this->jack_client);
        fprintf (stderr, "unique name `%s' assigned\n", client_name);
    }
    jack_set_process_callback (this->jack_client, \
    (jack_callbacks::process), (void *) this);
    /* tell the JACK server to call `jack_shutdown()' if
     * it ever shuts down, either entirely, or if it
     * just decides to stop calling us.
     * */
    jack_on_shutdown (this->jack_client,\
    jack_callbacks::shutdown, (void *) this);

    jack_port_t * input_port;
    jack_port_t * output_port;
    /* create two ports */
    input_port = jack_port_register (this->jack_client, "in",
                                     JACK_DEFAULT_AUDIO_TYPE,
                                     JackPortIsInput, 0);
    output_port = jack_port_register (this->jack_client, "out",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
    if ((input_port == nullptr) || (output_port == nullptr)) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
    else{
        input_ports.push_back(input_port);
        output_ports.push_back(output_port);
    }
}

int client::process(jack_nframes_t, void *) {
    return 0;
}

void client::shutdown(void *) {

}

void client::registerInPort(char *) {

}

void client::registerOutPort(char *) {

}

void client::unregisterInPort(char *) {

}

void client::unregisterOutPort(char *) {

}
