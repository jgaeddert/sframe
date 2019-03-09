// slot frame generator

#include "sframesync.hh"

sframesync::sframesync(unsigned int _payload_len) :
    sframe(_payload_len),
    detector(this)
{
}

sframesync::~sframesync()
{
}

void * sframesync::receive(const std::complex<float> * _buf)
{
    sdetect::results results = detector.execute(_buf);
    
    printf("results:\n");
    printf("  rms      = %12.8f\n", results.rms);
    printf("  rxy      = %12.8f\n", results.rxy);
    printf("  tau_hat  = %12.8f\n", results.tau_hat);
    printf("  dphi_hat = %12.8f\n", results.dphi_hat);
    printf("  phi_hat  = %12.8f\n", results.phi_hat);

    return NULL;
}
