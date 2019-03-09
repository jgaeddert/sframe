// slot detector

#ifndef __SDETECT_HH__
#define __SDETECT_HH__

#include <complex>
#include <liquid/liquid.h>
#include <fftw3.h>
#include "sframe.hh"

class sdetect
{
  public:
    /*! @brief create slot reference detector
     */
    sdetect(const sframe * _ref);
    ~sdetect();

    /*! @brief detection results */
    struct results {
        float rms;      // signal RMS
        float rxy;      // cross-correlator peak
        float tau_hat;  // timing offset estimate [samples]
        float dphi_hat; // carrier frequency offset estimate
        float phi_hat;  // carrier phase offset estimate
    };

    /*! @brief  run detection on reference symbols
     *  @param  _buf : buffer of raw received samples
     *  @return pointer to output decoded frame, NULL if invalid
     */
    results execute(std::complex<float> * _buf);

  protected:
    // fft for detection, time/freq buffers, etc.
    unsigned int            nfft;       ///< FFT size
    std::complex<float> *   R;          ///< reference sequence (freq domain)
    std::complex<float> *   buf_time;   ///< time-domain buffer
    std::complex<float> *   buf_freq;   ///< freq-domain buffer
    fftwf_plan              fft;        ///< fft object (forward)
    fftwf_plan              ifft;       ///< fft object (reverse)
};

#endif // __SDETECT_HH__
