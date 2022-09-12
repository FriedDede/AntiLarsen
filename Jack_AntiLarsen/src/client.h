//
// Created by andre on 9/12/22.
//

#ifndef JACK_ANTILARSEN_CLIENT_H
#define JACK_ANTILARSEN_CLIENT_H

#include <jack/jack.h>
#include <jack/types.h>
#include <vector>

namespace jack_callbacks {
    int process(jack_nframes_t,void *);
    void shutdown(void*);
}

class client {
public:
    explicit client(char*);
    int process(jack_nframes_t,void*);
    void shutdown(void*);
    void registerInPort (char *);
    void registerOutPort (char *);
    void unregisterInPort (char *);
    void unregisterOutPort (char *);
    void activate();
private:
    std::vector<jack_port_t *> input_ports;
    std::vector<jack_port_t *> output_ports;
    jack_client_t *jack_client;
    jack_options_t jack_options = JackNullOption;
    jack_status_t jack_status;
};


#endif
