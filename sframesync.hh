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

    /*! @brief  receive frame within slot
     *  @param  _buf : buffer of raw received samples
     *  @return pointer to output decoded frame, NULL if invalid
     */
    void * receive(const std::complex<float> * _buf);

    /*! @brief get timing offset estimate from last slot */
    float get_timing_offset() { return 0; }

  protected:
    sdetect         detector;       ///< reference symbol detector
    nco_crcf        mixer;          ///< mixer for coarse carrier frequency/phase recovery
    unsigned int    npfb;           ///< number of poly-phase filters in bank
    firpfb_crcf     mf;             ///< poly-phase filter-bank for matched filter & timing recovery
    qpilotsync      sync;           ///< frame recovery
    unsigned char * payload_rec;    ///< recovered payload bytes
};

#endif // __SFRAMESYNC_HH__

