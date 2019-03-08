// slot frame synchronizer

#ifndef __SFRAMESYNC_HH__
#define __SFRAMESYNC_HH__

#include <complex>
#include <liquid/liquid.h>
#include "sframe.hh"

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
    void * receive(std::complex<float> * _buf);

    /*! @brief get timing offset estimate from last slot */
    float get_timing_offset() { return 0; }

  protected:

    // fft for detection, time/freq buffers, etc.
};

#endif // __SFRAMESYNC_HH__

