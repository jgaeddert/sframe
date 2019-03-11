// slot frame synchronizer

#ifndef __SFRAMESYNC_HH__
#define __SFRAMESYNC_HH__

#include <complex>
#include <liquid/liquid.h>
#include "sframe.hh"
#include "sdetect.hh"

class sframesync : public sframe
{
  public:
    /*! @brief create slot frame generator
     */
    sframesync(unsigned int _payload_len);
    ~sframesync();

    /*! @brief reciever results */
    struct results {
        results();
        results(sdetect::results &);
        void print();

        const unsigned char *       payload;    ///< recovered payload bytes
        unsigned int                payload_len;///< length of payload [bytes]
        bool                        crc_pass;   ///< flag indicating if payload was received correctly

        float                       rssi;       ///< signal received signal strength indication [dB]
        float                       rxy;        ///< cross-correlator peak
        float                       tau_hat;    ///< timing offset estimate [samples]
        float                       dphi_hat;   ///< carrier frequency offset estimate [radians/sample]
        float                       evm;        ///< receiver error vector magnitude
        const std::complex<float> * syms;       ///< recovered payload symbols before demodulation
        unsigned int                num_syms;   ///< number of symbols
    };

    /*! @brief set threshold for detection; synchronizer won't attempt to decode below this value */
    void set_threshold(float _rxy_threshold) { rxy_threshold = _rxy_threshold; }

    /*! @brief  receive frame within slot
     *  @param  _buf : buffer of raw received samples
     *  @return pointer to output decoded frame, NULL if invalid
     */
    results receive(const std::complex<float> * _buf);

  protected:
    sdetect         detector;       ///< reference symbol detector
    float           rxy_threshold;  ///< threshold for which decoder need to attempt decoding
    nco_crcf        mixer;          ///< mixer for coarse carrier frequency/phase recovery
    unsigned int    npfb;           ///< number of poly-phase filters in bank
    firpfb_crcf     mf;             ///< poly-phase filter-bank for matched filter & timing recovery
    qpilotsync      sync;           ///< frame recovery
    unsigned char * payload_rec;    ///< recovered payload bytes
};

#endif // __SFRAMESYNC_HH__

