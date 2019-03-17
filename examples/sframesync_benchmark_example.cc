#include <time.h>
#include "sframesync.hh"

int main() {
    // options
    unsigned int payload_len = 256; // number of bytes in payload
    float        runtime_min = 5;   // minimum run time
    clockid_t    clockid     = CLOCK_REALTIME;

    // create objects
    sframesync sync = sframesync(payload_len);

    // number of samples in slot
    unsigned int num_samples = sync.get_slot_len();

    // temporary buffer for samples (noise)
    std::complex<float> buf[num_samples];
    for (unsigned int i=0; i<num_samples; i++)
        buf[i] = std::complex<float>(randnf(), randnf());

    // run trials
    struct timespec tic, toc;
    clock_gettime(clockid, &tic);
    float runtime = 0;
    unsigned int num_slots = 0;
    printf("running for %.2f seconds...\n", runtime_min);
    while (runtime < runtime_min) {
        // run over a block of slots
        for (unsigned int i=0; i<250; i++) {
            // run through detector
            //sframesync::results r =
            sync.receive(buf);
            num_slots++;
        }

        // update run time
        clock_gettime(clockid, &toc);
        runtime = toc.tv_sec - tic.tv_sec + 1e-9*(toc.tv_nsec - tic.tv_nsec);
    }

    // display results and return
    printf("processed %u slots (%u samples each) in %.3f seconds\n", num_slots, num_samples, runtime);
    printf("  slot rate         : %.3f slots/second\n",   (float)(num_slots            ) / runtime);
    printf("  sampling rate     : %.3f samples/second\n", (float)(num_slots*num_samples) / runtime);
    return 0;
}

