// test timing bias in estimator
#include "sframegen.hh"
#include "sframesync.hh"

struct results {
    float tau;        // actual time offset
    float tau_avg;    // average estimate
    float tau_rmse;   // root mean-squared error
    void  print() { printf("  %12.8f %12.8f %12.4e\n", tau, tau_avg, tau_rmse); }
};

results run_batch(unsigned int _payload_len,
                  unsigned int _num_trials,
                  float        _tau);

int main() {
    // options
    unsigned int payload_len =    64;   // number of bytes in payload
    float        tau_min     = -1.0f;   // starting timing offset [samples]
    float        tau_max     =  1.0f;   // ending timing offset [samples]
    unsigned int num_steps   =   101;   // number of timing offset steps
    unsigned int num_trials  =   100;   // number of trials to run

    printf("# %12s %12s %12s\n", "tau", "bias", "rmse");
    float tau_step = (tau_max - tau_min) / (float)(num_steps-1);
    for (unsigned int i=0; i<num_steps; i++) {
        float tau = tau_min + i*tau_step;
        results r = run_batch(payload_len, num_trials, tau);
        r.print();
    }
    return 0;
}

results run_batch(unsigned int _payload_len,
                  unsigned int _num_trials,
                  float        _tau)
{
    // create objects
    sframegen  gen  = sframegen (_payload_len);
    sframesync sync = sframesync(_payload_len);

    // generate buffers
    unsigned int buf_len = gen.get_slot_len();
    std::complex<float> buf_channel[buf_len];

    // delay filter
    unsigned int m      = 15;               // delay filter semi-length
    int          d      = (int)roundf(_tau); // integer sample delay
    float        mu     = _tau - (float)d;   // fractional sample delay
    firfilt_crcf fdelay = firfilt_crcf_create_kaiser(2*m+1, 0.4f, 60.0f, -mu);

    // run trials
    float tau_rmse = 0.0f;  // timing root mean-squared error
    float tau_avg  = 0.0f;  // average timing estimate (bias)
    for (unsigned int t=0; t<_num_trials; t++) {
        // generate frame with random payload
        const std::complex<float> * buf = gen.generate();
        firfilt_crcf_reset(fdelay);

        // run through channel
        unsigned int n = 0;
        for (unsigned int i=0; i<buf_len+m-d; i++) {
            // push sample into filter and save results at appropriate indices
            firfilt_crcf_push(fdelay, i < buf_len ? buf[i] : 0);
            if (n < buf_len) firfilt_crcf_execute(fdelay, &buf_channel[n]);
            if (i >= m-d)    n++;
        }

        // run through detector and get results
        sframesync::results r = sync.receive(buf_channel);
        tau_avg  += r.tau_hat;
        tau_rmse += (_tau - r.tau_hat) * (_tau - r.tau_hat);
    }
    firfilt_crcf_destroy(fdelay);

    // populate and return results
    return {.tau = _tau,
            .tau_avg  = tau_avg / (float)_num_trials,
            .tau_rmse = sqrtf(tau_rmse/(float)_num_trials) };
}

