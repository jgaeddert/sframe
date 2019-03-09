// slot frame reference detector

#include "sdetect.hh"

sdetect::sdetect(const sframe * _ref)
{
    // TODO: adjust fft size appropriately
    unsigned int k = 2;
    unsigned int m = std::min(_ref->num_symbols_guard, 9U);
    unsigned int p = _ref->num_symbols_ref + 2*_ref->num_symbols_guard;
    nfft = 2 << (unsigned int)(roundf(liquid_nextpow2(k*p)));
    nfft = std::min(nfft, _ref->num_samples_slot); // ensure we don't observe more than the slot time

    // TODO: allocate with fft allocation methods
    R        = new std::complex<float>[nfft];
    buf_time = new std::complex<float>[nfft];
    buf_freq = new std::complex<float>[nfft];

    // create transform objects
    fft  = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_time), reinterpret_cast<fftwf_complex*>(buf_freq), FFTW_FORWARD,  FFTW_ESTIMATE);
    ifft = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_freq), reinterpret_cast<fftwf_complex*>(buf_time), FFTW_BACKWARD, FFTW_ESTIMATE);

    // create (temporary) square-root nyquist interpolator with 2 samples/symbol
    firinterp_crcf interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER, 2, m, 0.25f, 0.0f);

    // first guard period, compensating for filter delay
    unsigned int n=0;
    for (unsigned int i=0; i<_ref->num_symbols_guard - m; i++)
        firinterp_crcf_execute(interp, 0, &buf_time[2*n++]);

    // reference block
    for (unsigned int i=0; i<_ref->num_symbols_ref; i++)
        firinterp_crcf_execute(interp, _ref->syms_ref[i], &buf_time[2*n++]);

    // flush with zeros
    while (n < nfft/k)
        firinterp_crcf_execute(interp, 0, &buf_time[2*n++]);

    // clean up temporary objects
    firinterp_crcf_destroy(interp);

    // run transform on reference and copy
    fftwf_execute(fft);
    memmove(R, buf_freq, nfft*sizeof(std::complex<float>));

    // compute signal level
    ref2 = liquid_sumsqcf(buf_time, nfft);
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

sdetect::results sdetect::execute(const std::complex<float> * _buf)
{
    // create results object to return
    sdetect::results results = {
        .rms        = 0,
        .rxy        = 0,
        .tau_hat    = 0,
        .dphi_hat   = 0,
        .phi_hat    = 0,};

    // copy input to time-domain buffer
    memmove(buf_time, _buf, nfft*sizeof(std::complex<float>));

    // compute...
    results.rms = std::sqrt( liquid_sumsqcf(buf_time, nfft) / (float)nfft );

    // run transform: buf_time -> buf_freq
    fftwf_execute(fft);

    // cross-multiply with expected signal
    for (unsigned int i=0; i<nfft; i++)
        buf_freq[i] *= std::conj(R[i]);

    // run inverse transform: buf_freq -> buf_time
    fftwf_execute(ifft);

    // find peak index of time-domain signal
    float        vmax = 0;
    unsigned int imax = 0;
    for (unsigned int i=0; i<nfft; i++) {
        float v = std::abs(buf_time[i]);
        if (i==0 || v > vmax) {
            vmax = v;
            imax = i;
        }
    }

    // normalize peak
    float g = 1.0f / ( (float)nfft * ref2 * results.rms );
    results.rxy = vmax * g;

#if 0
    // save results to file
    FILE * fid = fopen("sdetect.dat","w");
    for (unsigned int i=0; i<nfft; i++)
        fprintf(fid,"%12.4e %12.4e\n", g*buf_time[i].real(), g*buf_time[i].imag());
    fclose(fid);
#endif

    return results;
}
