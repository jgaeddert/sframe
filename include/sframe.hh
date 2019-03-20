// slot frame base class

#ifndef __SFRAME_HH__
#define __SFRAME_HH__

#include <complex>
#include <liquid/liquid.h>

class sdetect;

/*! base class with common parameters for slot frames
 */
class sframe
{
  friend class sdetect;
  public:
    /*! @brief slot frame base class
     *  @param _payload_len : length of payload [bytes]
     *  @param _check       : error detection scheme
     *  @param _fec0        : outer forward error-correction scheme
     *  @param _fec1        : inner forward error-correction scheme
     *  @param _ms          : linear modulation scheme
     */
    sframe(unsigned int _payload_len,
           crc_scheme   _check = LIQUID_CRC_32,
           fec_scheme   _fec0  = LIQUID_FEC_NONE,
           fec_scheme   _fec1  = LIQUID_FEC_NONE,
           int          _ms    = LIQUID_MODEM_QPSK);
    ~sframe();

    // accessor methods
    unsigned int get_payload_len() { return payload_len; }
    unsigned int get_slot_len()    { return num_samples_slot; }

    /*! print basic information about object to file */
    virtual void print(FILE * _fid = stdout);

  protected:

    // frame lengths
    unsigned int    payload_len;            ///< length of payload (bytes)
    unsigned int    num_symbols_payload;    ///< number of modulated symbols in qpacketmodem
    unsigned int    pilot_spacing;          ///< spacing between pilot symbols
    unsigned int    num_symbols_frame;      ///< number of modulated symbols with pilots added
    unsigned int    num_symbols_ref;        ///< number of reference symbols at front/tail
    unsigned int    num_symbols_guard;      ///< number of guard symbols at front/tail
    unsigned int    num_symbols_slot;       ///< total number of symbols in slot
    unsigned int    k;                      ///< filter samples per symbol
    unsigned int    m;                      ///< filter semi-length
    float           beta;                   ///< filter excess bandwidth factor
    unsigned int    num_samples_slot;       ///< length of slot (samples)

    // buffers
    std::complex<float> * syms_ref;     ///< reference symbols at head of slot
    std::complex<float> * syms_payload; ///< payload symbols (e.g. data)
    std::complex<float> * syms_frame;   ///< frame symbols with pilots included
    std::complex<float> * buf_slot;     ///< entire slot buffer [samples]

    // common objects
    qpacketmodem mod;                   ///< combined modulation/FEC encoder
};

#endif // __SFRAME_HH__

