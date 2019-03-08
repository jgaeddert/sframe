#include "sframegen.hh"
#include "sframesync.hh"

int main() {
    // options
    unsigned int payload_len = 256;
    const char * filename    = "sframe_example.dat";

    // create objects
    sframegen  gen  = sframegen (payload_len);
    sframesync sync = sframesync(payload_len);

    // number of samples in slot
    unsigned int num_samples = gen.get_slot_len();

    gen.print();
    sync.print();

    // generate payload data
    unsigned char payload[payload_len];
    for (unsigned int i=0; i<payload_len; i++)
        payload[i] = rand() & 0xff;

    // generate frame
    const std::complex<float> * buf = gen.generate(payload);

    // save samples to output file
    FILE * fid = fopen(filename,"w");
    for (unsigned int i=0; i<num_samples; i++)
        fprintf(fid,"  %12.8f %12.8f\n", buf[i].real(), buf[i].imag());
    fclose(fid);
    printf("results written to %s\n", filename);

    return 0;
}

