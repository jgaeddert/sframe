// slot frame generator

#include <string.h>
#include "sframegen.hh"

sframegen::sframegen(unsigned int _payload_len,
                     crc_scheme   _check,
                     fec_scheme   _fec0,
                     fec_scheme   _fec1,
                     int          _ms) :
    sframe(_payload_len, _check, _fec0, _fec1, _ms)
{
    // create object to add pilots for phase recovery
    gen = qpilotgen_create(num_symbols_payload, pilot_spacing);

    // create square-root nyquist interpolator with 2 samples/symbol
    interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER, k, m, beta, 0.0f);
}

sframegen::~sframegen()
{
    // destroy internal objects
    qpilotgen_destroy(gen);
    firinterp_crcf_destroy(interp);
}

const std::complex<float> * sframegen::generate(unsigned char * _payload)
{
    // encode frame symbols
    qpacketmodem_encode(mod, _payload, syms_payload);

    // add pilot symbols
    qpilotgen_execute(gen, syms_payload, syms_frame);

    // interpolate result
    firinterp_crcf_reset(interp);
    unsigned int n = 0;

    // first guard period, compensating for filter delay
    for (unsigned int i=0; i<num_symbols_guard - m; i++)
        firinterp_crcf_execute(interp, 0, &buf_slot[k*n++]);

    // reference block
    for (unsigned int i=0; i<num_symbols_ref; i++)
        firinterp_crcf_execute(interp, syms_ref[i], &buf_slot[k*n++]);

    // payload symbols
    for (unsigned int i=0; i<num_symbols_frame; i++)
        firinterp_crcf_execute(interp, syms_frame[i], &buf_slot[k*n++]);

    // last guard period, flushing filter
    for (unsigned int i=0; i<num_symbols_guard + m; i++)
        firinterp_crcf_execute(interp, 0, &buf_slot[k*n++]);

    //assert(n == num_symbols_slot);
    return buf_slot;
}
