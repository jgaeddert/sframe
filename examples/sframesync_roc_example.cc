// test receiver operating characteristic (ROC)
#include <iostream>
#include "sframegen.hh"
#include "sframesync.hh"

int main() {
    // options
    unsigned int payload_len =    64;   // number of bytes in payload
    unsigned int num_trials  =  2000;   // number of trials to run
    float        SNRdB       =  0.0f;   // nominal SNR when signal is present
    std::string  filename    = "sframesync_roc_example.dat";

    // create objects
    sframegen  gen  = sframegen (payload_len);
    sframesync sync = sframesync(payload_len);

    // generate buffers
    unsigned int buf_len = gen.get_slot_len();
    std::complex<float> buf_channel[buf_len];
    unsigned char payload[payload_len];

    float nstd = powf(10.0f, -SNRdB/20.0f) * M_SQRT1_2;

    float rxy_0[num_trials];    // no signal present
    float rxy_1[num_trials];    // signal present

    // run trials
    sframesync::results r;
    FILE * fid = fopen(filename.c_str(),"w");
    fprintf(fid,"# %12s %12s\n", "no signal", "signal");
    for (unsigned int t=0; t<num_trials; t++) {
        // generate noise
        for (unsigned int i=0; i<buf_len; i++)
            buf_channel[i] = std::complex<float>(randnf(),randnf()) * nstd;

        // run through detector with signal absent
        r = sync.receive(buf_channel);
        rxy_0[t] = r.rxy;

        // generate payload data
        for (unsigned int i=0; i<payload_len; i++)
            payload[i] = rand() & 0xff;

        // generate frame
        const std::complex<float> * buf = gen.generate(payload);

        // add signal onto noise
        for (unsigned int i=0; i<buf_len; i++)
            buf_channel[i] += buf[i];

        // run through detector with signal present
        r = sync.receive(buf_channel);
        rxy_1[t] = r.rxy;

        // print results to screen
        fprintf(fid,"  %12.8f %12.8f\n", rxy_0[t], rxy_1[t]);
    }
    fclose(fid);
    std::cout << "results written to " << filename << std::endl;
}

