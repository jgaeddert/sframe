// slot frame generator

#ifndef __SFRAMEGEN_HH__
#define __SFRAMEGEN_HH__

#include <complex>
#include <liquid/liquid.h>
#include "sframe.hh"

class sframegen : public sframe
{
  public:
    /*! @brief create slot frame generator
     */
    sframegen(unsigned int _payload_len,
              crc_scheme   _check = LIQUID_CRC_32,
              fec_scheme   _fec0  = LIQUID_FEC_NONE,
              fec_scheme   _fec1  = LIQUID_FEC_NONE,
              int          _ms    = LIQUID_MODEM_QPSK);
    ~sframegen();

    /*! @brief generate frame and pass pointer to time-domain samples for transmission
     *  @param  _payload : input buffer of payload bytes
     *  @return pointer to output buffer of samples
     */
    const std::complex<float> * generate(unsigned char * _payload);

  protected:
    qpilotgen      gen;     ///< pilot injection
    firinterp_crcf interp;  ///< square-root Nyquist matched filter interpolator
};

#endif // __SFRAMEGEN_HH__

