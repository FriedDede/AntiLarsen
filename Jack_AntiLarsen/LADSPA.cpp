//
// Created by andre on 5/7/22.
// LADSPA integration handlers
//
#include <iostream>
#include "include/iir.h"
#include "include/Analyzer.h"
#include "include/localtools.h"
#include "ladspa.h"
#include <cstdlib>

static LADSPA_Descriptor * g_psDescriptors;
static preFiltersBank *bank = nullptr;
static activeFilters *filters = nullptr;
static peaksFinder  *analyzer = nullptr;
static bool runnable = false;

/*
 * LADSPA instantiate handle
 */
static LADSPA_Handle instantiate(const LADSPA_Descriptor *,unsigned long f_sampling) {
    bool setting[3] = {true,true,true};
    bank = new preFiltersBank((float)f_sampling,-10,30,0);
    filters = new activeFilters;
    analyzer = new peaksFinder(nullptr, setting);
}
/*
 * LADSPA Connect ports handle:
 * Filters bank in
 * Filters bank out
 * Analyzer In
 */
static void connectPort(LADSPA_Handle Instance, unsigned long Port, LADSPA_Data * DataLocation) {
    switch (Port) {
        case 0:
            if (filters != nullptr) filters->setIn(DataLocation);
            break;
        case 1:
            if (filters != nullptr) filters->setOut(DataLocation);
            break;
        case 2:
            if (analyzer != nullptr) analyzer->setBuffer(DataLocation);
            break;
        default:
            break;
    }
}
/*
 * LADSPA activate filter handle
 */
static void activate(void * pvHandle){
    runnable = true;
}
/*
 * LADSPA filter routine
 */
static void run(LADSPA_Handle Instance,unsigned long SampleCount){
    if (runnable){
        if (analyzer != nullptr) analyzer->run();
        else return;
        if (bank != nullptr and filters != nullptr) {
            for (auto &f : analyzer->found_howls) {
                filters->add_filter_to_bank(f,bank->filters);
            }
            filters->applyFilters();
        }
    }
}
/*
 * LADSPA filter cleanup
 */
static void cleanup(void *pvHandle) {
    delete bank;
    delete filters;
    delete analyzer;
}

/* Global object used handle startup initialisation and shut down
   tidying. Performs the function of the _init() and _fini() calls in
   the C modules. */
class StartupShutdownHandler {
public:
    StartupShutdownHandler() {

        char ** pcPortNames;
        LADSPA_PortDescriptor * piPortDescriptors;
        LADSPA_PortRangeHint * psPortRangeHints;
        g_psDescriptors  = new LADSPA_Descriptor;

        if (g_psDescriptors  == nullptr) exit(EXIT_FAILURE);

        g_psDescriptors ->UniqueID = 1044;
        g_psDescriptors ->Properties = LADSPA_PROPERTY_HARD_RT_CAPABLE;
        g_psDescriptors ->Maker = localStrdup("Andrea Motta");
        g_psDescriptors ->Copyright = localStrdup("None");
        g_psDescriptors ->PortCount = 3;
        piPortDescriptors = new LADSPA_PortDescriptor[3];
        g_psDescriptors ->PortDescriptors = (const LADSPA_PortDescriptor *)piPortDescriptors;
        piPortDescriptors[1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[0] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[2] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        pcPortNames = new char *[3];
        g_psDescriptors ->PortNames = (const char **)pcPortNames;
        pcPortNames[0] = localStrdup("Input:");
        pcPortNames[1] = localStrdup("Output:");
        pcPortNames[2] = localStrdup("Analyzer Input:");
        psPortRangeHints = new LADSPA_PortRangeHint[3];
        g_psDescriptors ->PortRangeHints = (const LADSPA_PortRangeHint *)psPortRangeHints;
        psPortRangeHints[0].HintDescriptor = 0;
        psPortRangeHints[1].HintDescriptor = 1;
        psPortRangeHints[2].HintDescriptor = 2;
        // LADSPA global descriptor
        g_psDescriptors ->instantiate = instantiate;
        g_psDescriptors ->connect_port = connectPort;
        g_psDescriptors ->activate = activate;
        g_psDescriptors ->run_adding = nullptr;
        g_psDescriptors ->set_run_adding_gain = nullptr;
        g_psDescriptors ->deactivate = nullptr;
        g_psDescriptors ->cleanup = cleanup;
        g_psDescriptors ->Label = localStrdup("FDBK-KILLER");
        g_psDescriptors ->Name = localStrdup("RT Feedback Suppressor");
        g_psDescriptors ->run = run;
    }

    void deleteDescriptor(LADSPA_Descriptor * psDescriptor) {
        unsigned long lIndex;
        if (psDescriptor) {
            delete [] psDescriptor->Label;
            delete [] psDescriptor->Name;
            delete [] psDescriptor->Maker;
            delete [] psDescriptor->Copyright;
            delete [] psDescriptor->PortDescriptors;
            for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
                delete [] psDescriptor->PortNames[lIndex];
            delete [] psDescriptor->PortNames;
            delete [] psDescriptor->PortRangeHints;
            delete psDescriptor;
        }
    }

    ~StartupShutdownHandler() {
        deleteDescriptor(g_psDescriptors);
    }

};

static StartupShutdownHandler g_oShutdownStartupHandler;

const LADSPA_Descriptor *ladspa_descriptor() {
    return g_psDescriptors;
}
