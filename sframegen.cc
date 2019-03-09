// slot frame generator

#include <string.h>
#include "sframegen.hh"

sframegen::sframegen(unsigned int _payload_len) :
    sframe(_payload_len)
{
    // create object to add pilots for phase recovery
    gen = qpilotgen_create(num_symbols_payload, pilot_spacing);

    // compute filter semi-length
    m = std::min(num_symbols_guard, 9U);

    // create square-root nyquist interpolator with 2 samples/symbol
    interp = firinterp_crcf_create_prototype(LIQUID_FIRFILT_ARKAISER, 2, m, 0.25f, 0.0f);
}

sframegen::~sframegen()
{
    // destroy internal objects
    qpilotgen_destroy(gen);
    firinterp_crcf_destroy(interp);
}

const std::complex<float> * sframegen::generate(unsigned char * _payload)
{
    // reset buffer
    memset(buf_slot, 0x00, num_samples_slot*sizeof(std::complex<float>));

    // encode frame symbols
    qpacketmodem_encode(mod, _payload, syms_payload);

    // add pilot symbols
    qpilotgen_execute(gen, syms_payload, syms_frame);

    // interpolate result
    firinterp_crcf_reset(interp);
    unsigned int i;
    unsigned int n = 0;

    // first guard period, compensating for filter delay
    for (i=0; i<num_symbols_guard - m; i++)
        firinterp_crcf_execute(interp, 0, &buf_slot[2*n++]);

    // first reference block
    for (i=0; i<num_symbols_ref; i++)
        firinterp_crcf_execute(interp, syms_ref[i], &buf_slot[2*n++]);

    // payload symbols
    for (i=0; i<num_symbols_frame; i++)
        firinterp_crcf_execute(interp, syms_frame[i], &buf_slot[2*n++]);

    // last guard period, flushing filter
    for (i=0; i<num_symbols_guard + m; i++)
        firinterp_crcf_execute(interp, 0, &buf_slot[2*n++]);

    //assert(n == num_symbols_slot);
    return buf_slot;
}
