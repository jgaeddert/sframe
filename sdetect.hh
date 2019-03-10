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

    /*! @brief set (normalized) frequency offset range to search over
     *  @param _freq_range : frequency range to search over, relative to sample rate
     */
    void set_frequency_offset_range(float _freq_range);

    /*! @brief  run detection on reference symbols
     *  @param  _buf : buffer of raw received samples
     *  @return pointer to output decoded frame, NULL if invalid
     */
    results execute(const std::complex<float> * _buf);

  protected:
    // fft for detection, time/freq buffers, etc.
    unsigned int            nfft;       ///< FFT size
    std::complex<float> *   R;          ///< reference sequence (freq domain)
    std::complex<float> *   buf_time;   ///< time-domain buffer
    std::complex<float> *   buf_freq_0; ///< freq-domain buffer
    std::complex<float> *   buf_freq_1; ///< freq-domain buffer
    fftwf_plan              fft;        ///< fft object (forward)
    fftwf_plan              ifft;       ///< fft object (reverse)
    float                   ref2;       ///< sum of squares for reference level (time domain)
    int                     range;      ///< frequency offset range index
};

#endif // __SDETECT_HH__

