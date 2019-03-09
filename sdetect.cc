// slot frame reference detector

#include "sdetect.hh"

sdetect::sdetect(const sframe * _ref)
{
    // TODO: adjust fft size appropriately
    unsigned int k = 2;
    unsigned int m = std::min(_ref->num_symbols_guard, 9U);
    unsigned int n = _ref->num_symbols_ref + 2*_ref->num_symbols_guard;
    nfft = 2 << (unsigned int)(roundf(liquid_nextpow2(k*n)));

    // TODO: allocate with fft allocation methods
    R        = new std::complex<float>[nfft];
    buf_time = new std::complex<float>[nfft];
    buf_time = new std::complex<float>[nfft];

    // create transform objects
    fft  = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_time), reinterpret_cast<fftwf_complex*>(buf_freq), FFTW_FORWARD,  FFTW_ESTIMATE);
    ifft = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_freq), reinterpret_cast<fftwf_complex*>(buf_time), FFTW_BACKWARD, FFTW_ESTIMATE);
}

sdetect::~sdetect()
{
    // destroy FFT objects
    fftwf_destroy_plan( fft);
    fftwf_destroy_plan(ifft);

    // free allocated memory
    delete [] R;
    delete [] buf_time;
    delete [] buf_freq;
}

sdetect::results sdetect::execute(std::complex<float> * _buf)
{
    // create results object to return
    sdetect::results r = {
        .rms        = 0,
        .rxy        = 0,
        .tau_hat    = 0,
        .dphi_hat   = 0,
        .phi_hat    = 0,};

    return r;
}
