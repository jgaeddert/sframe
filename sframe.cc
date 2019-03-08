// slot frame base class

#include "sframe.hh"

sframe::sframe(unsigned int _payload_len) :
    payload_len(_payload_len)
{
    // create and configure mod/fec object
    mod = qpacketmodem_create();
    qpacketmodem_configure(mod,
        payload_len, LIQUID_CRC_32, LIQUID_FEC_NONE, LIQUID_FEC_NONE, LIQUID_MODEM_QPSK);

    // frame lengths
    num_symbols_frame   = qpacketmodem_get_frame_len(mod);
    num_symbols_ref     = 80;
    num_symbols_guard   = 20;
    num_symbols_slot    = num_symbols_frame + 2*num_symbols_ref + 2*num_symbols_guard;
    num_samples_slot    = 2 * num_symbols_slot;

    // buffers
    syms_ref_0 = new std::complex<float>[num_symbols_ref  ];
    syms_ref_1 = new std::complex<float>[num_symbols_ref  ];
    syms_frame = new std::complex<float>[num_symbols_frame];
    buf_slot   = new std::complex<float>[num_samples_slot ];

    // generate pseudo-random reference symbol sequence
    msequence ms = msequence_create_default(8);
    float     s2 = M_SQRT1_2;
    for (unsigned int i=0; i<num_symbols_ref; i++) {
        // generate a pair of QPSK symbol indices
        unsigned int sym_0 = msequence_generate_symbol(ms, 2);
        unsigned int sym_1 = msequence_generate_symbol(ms, 2);

        // modulate into reference symbols
        syms_ref_0[i] = std::complex<float>(sym_0 & 1 ? s2 : -s2, sym_0 & 2 ? s2 : -s2);
        syms_ref_1[i] = std::complex<float>(sym_1 & 1 ? s2 : -s2, sym_1 & 2 ? s2 : -s2);
    }
    msequence_destroy(ms);
}

sframe::~sframe()
{
    // free buffers
    delete [] syms_ref_0;
    delete [] syms_ref_1;
    delete [] syms_frame;
    delete [] buf_slot;

    // destroy mod/fec object
    qpacketmodem_destroy(mod);
}

void sframe::print(FILE * _fid)
{
    fprintf(_fid,"sframe: payload=%u bytes, frame=%u samples\n",
            payload_len, num_samples_slot);
}

