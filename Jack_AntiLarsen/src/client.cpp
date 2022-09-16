//
// Created by andre on 9/12/22.
//

#include "../include/client.h"
#include <jack/jack.h>
#include <jack/types.h>
#include <iostream>

int jack_callbacks::process(jack_nframes_t nframes, void *clientpointer) {
    auto *cp = reinterpret_cast<class client *>(clientpointer);
    if(cp->enabled) cp->process(nframes,nullptr);
    return 0;
}

void jack_callbacks::shutdown(void *clientpointer) {
    auto *cp = reinterpret_cast<class client *>(clientpointer);
    delete cp;
}

client::client(const char *client_name) {

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
    jack_on_shutdown (this->jack_client,\
    jack_callbacks::shutdown, (void *) this);

    this->input_port = jack_port_register (this->jack_client, "in",
                                     JACK_DEFAULT_AUDIO_TYPE,
                                     JackPortIsInput, 0);
    this->output_port = jack_port_register (this->jack_client, "out",
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
    if ((input_port == nullptr) || (output_port == nullptr)) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
}

int client::process(jack_nframes_t, void *) {
    return 0;
}

void client::registerInPort(const char *port_name) {

    this->input_port = jack_port_register (this->jack_client, port_name,
                                     JACK_DEFAULT_AUDIO_TYPE,
                                     JackPortIsInput, 0);
    if (this->input_port == nullptr) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
}

void client::registerOutPort(const char *port_name) {

    this->output_port = jack_port_register (this->jack_client, port_name,
                                      JACK_DEFAULT_AUDIO_TYPE,
                                      JackPortIsOutput, 0);
    if (this->output_port == nullptr) {
        fprintf(stderr, "no more JACK ports available\n");
        exit (1);
    }
}

void client::unregisterPort(const char *port_name) {
    jack_port_unregister(this->jack_client, \
    jack_port_by_name(this->jack_client,port_name));
}


void client::activate() {
    if (jack_activate (this->jack_client) != EXIT_SUCCESS){
        fprintf (stderr, "cannot activate client");
        exit (1);
    }
}

jack_client_t *client::getJackClient() const {
    return jack_client;
}

client::~client() = default;
