// slot frame reference detector

#include "sdetect.hh"

sdetect::sdetect(const sframe * _ref)
{
    // TODO: adjust fft size appropriately
    unsigned int k = 2;
    unsigned int m = std::min(_ref->num_symbols_guard, 9U);
    unsigned int p = _ref->num_symbols_ref + 2*_ref->num_symbols_guard;
    nfft = 1 << (unsigned int)(roundf(liquid_nextpow2(k*p)));
    nfft = std::min(nfft, _ref->num_samples_slot); // ensure we don't observe more than the slot time

    // TODO: allocate with fft allocation methods
    R          = new std::complex<float>[nfft];
    buf_time   = new std::complex<float>[nfft];
    buf_freq_0 = new std::complex<float>[nfft];
    buf_freq_1 = new std::complex<float>[nfft];

    // create transform objects
    fft  = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_time  ), reinterpret_cast<fftwf_complex*>(buf_freq_0), FFTW_FORWARD,  FFTW_ESTIMATE);
    ifft = fftwf_plan_dft_1d(nfft, reinterpret_cast<fftwf_complex*>(buf_freq_1), reinterpret_cast<fftwf_complex*>(buf_time  ), FFTW_BACKWARD, FFTW_ESTIMATE);

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
    memmove(R, buf_freq_0, nfft*sizeof(std::complex<float>));

    // compute signal level
    ref2 = liquid_sumsqcf(buf_time, nfft);

    // set default frequency offset range
    set_frequency_offset_range(0.05f);
}

void sdetect::set_frequency_offset_range(float _freq_range)
{
    if (_freq_range < -0.5f || _freq_range > 0.5f)
        throw std::runtime_error("frequency offset range out of range");

    // compute range for searching for frequency offset
    range = (int)std::ceil( _freq_range * nfft / (2*M_PI) );
    printf("range = %d\n", range);
}

sdetect::~sdetect()
{
    // destroy FFT objects
    fftwf_destroy_plan( fft);
    fftwf_destroy_plan(ifft);

    // free allocated memory
    delete [] R;
    delete [] buf_time;
    delete [] buf_freq_0;
    delete [] buf_freq_1;
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

    // compute input signal RMS
    results.rms = std::sqrt( liquid_sumsqcf(buf_time, nfft) / (float)nfft );

    // run transform: buf_time -> buf_freq_0
    fftwf_execute(fft);

    // apply different frequency offset hypotheses
    float vpeak = 0;
    for (int p=-range; p<=range; p++) {

        // cross-multiply with expected signal shifted by frequency offset
        for (unsigned int i=0; i<nfft; i++)
            buf_freq_1[i] = buf_freq_0[i] * std::conj(R[(nfft+i-p)%nfft]);

        // run inverse transform: buf_freq_1 -> buf_time
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

        // compare max for this with global maximum
        if (vmax < vpeak)
            continue;

        // save peak
        vpeak = vmax;

        // normalize peak
        float g = 1.0f / ( (float)nfft * ref2 * results.rms );
        results.rxy = vmax * g;

        // compute timing offset (samples)
        // TODO: interpolate between available sample points
        float dt = quadratic_interpolation(
            std::abs(buf_time[(imax + nfft - 1) % nfft]),
            std::abs(buf_time[ imax                   ]),
            std::abs(buf_time[(imax + nfft + 1) % nfft]) );
        results.tau_hat = (imax < nfft/2 ? (float)imax : (float)imax - (float)nfft) + dt;

        // compute carrier phase offset estimate
        results.phi_hat = std::arg(buf_time[imax]);

        // compute carrier frequency offset estimate
        results.dphi_hat = 2*M_PI*(float)p / (float)nfft;

#if 0
        printf("  p = %3d, rxy = %12.8f, dphi-hat: %12.8f\n", p, results.rxy, results.dphi_hat);
        // save results to file
        FILE * fid = fopen("sdetect.dat","w");
        for (unsigned int i=0; i<nfft; i++)
            fprintf(fid,"%12.4e %12.4e\n", g*buf_time[i].real(), g*buf_time[i].imag());
        fclose(fid);
#endif

    }

    return results;
}

float sdetect::quadratic_interpolation(float _yneg, float _y0, float _ypos)
{
    if (_yneg > _y0 || _ypos > _y0)
        throw std::runtime_error("invalid bounds for quadratic interpolation");

    float a = 0.5f*(_ypos + _yneg) - _y0;
    float b = 0.5f*(_ypos - _yneg);
    //float c = _y0;
    return -b / (2.0f*a);
}
