#include "sframegen.hh"
#include "sframesync.hh"

int main() {
    // options
    unsigned int payload_len = 256;     // number of bytes in payload
    float        g           = 0.1f;    // channel gain
    int          dt          = -3;      // timing offset
    float        phi         = 0.50f;   // carrier phase offset
    float        dphi        = 0.02f;   // carrier frequency offset
    float        nstd        = 0.01f;   // noise standard deviation
    const char * filename    = "sframe_example.dat";

    // create objects
    sframegen  gen  = sframegen (payload_len);
    gen.print();
    sframesync sync = sframesync(payload_len);
    sync.print();

    // number of samples in slot
    unsigned int num_samples = gen.get_slot_len();

    // temporary buffer for channel
    std::complex<float> buf_channel[num_samples];

    // generate payload data
    unsigned char payload[payload_len];
    for (unsigned int i=0; i<payload_len; i++)
        payload[i] = rand() & 0xff;

    // generate frame
    const std::complex<float> * buf = gen.generate(payload);

    // run through channel
    for (unsigned int i=0; i<num_samples; i++) {
        std::complex<float> nco( std::cos(phi), std::sin(phi) );
        buf_channel[i] = g * nco * buf[(num_samples+i-dt)%num_samples] +
                         std::complex<float>(randnf(),randnf()) * float(nstd*M_SQRT1_2);

        // update nco phase and constrain to [-pi,pi)
        phi += dphi;
        while (phi < -M_PI) phi += 2*M_PI;
        while (phi >  M_PI) phi -= 2*M_PI;
    }

    // save samples to output file
    FILE * fid = fopen(filename,"w");
    for (unsigned int i=0; i<num_samples; i++) {
        fprintf(fid,"  %12.8f %12.8f %12.4e %12.4e\n",
                buf[i].real(), buf[i].imag(),
                buf_channel[i].real(), buf_channel[i].imag());
    }
    fclose(fid);
    printf("results written to %s\n", filename);

    // run through detector
    sync.receive(buf_channel);

    return 0;
}

