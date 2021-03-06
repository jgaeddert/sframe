// slot frame base class

#include "sframe.hh"

sframe::sframe(unsigned int _payload_len,
               crc_scheme   _check,
               fec_scheme   _fec0,
               fec_scheme   _fec1,
               int          _ms) :
    payload_len(_payload_len)
{
    // create and configure mod/fec object
    mod = qpacketmodem_create();
    qpacketmodem_configure(mod, payload_len, _check, _fec0, _fec1, _ms);

    // frame lengths
    num_symbols_payload = qpacketmodem_get_frame_len(mod);

    // compute appropriate spacing between pilots
    pilot_spacing       = 40; // TODO: make this dependent upon frame length
    while (qpilot_num_pilots(num_symbols_payload, pilot_spacing) < 4 && pilot_spacing > 2)
        pilot_spacing -= 2;
    num_symbols_frame   = qpilot_frame_len(num_symbols_payload, pilot_spacing);

    // static values
    num_symbols_ref     = 120;
    num_symbols_guard   = 20;
    num_symbols_slot    = num_symbols_frame + num_symbols_ref + 2*num_symbols_guard;
    k                   = 2;
    m                   = std::min(num_symbols_guard, 9U);
    beta                = 0.25f;
    num_samples_slot    = k * num_symbols_slot;

    // buffers
    syms_ref    = new std::complex<float>[num_symbols_ref    ];
    syms_payload= new std::complex<float>[num_symbols_payload];
    syms_frame  = new std::complex<float>[num_symbols_frame  ];
    buf_slot    = new std::complex<float>[num_samples_slot   ];

    // generate pseudo-random reference symbol sequence
    msequence ms = msequence_create_default(8);
    float     s2 = M_SQRT1_2;
    for (unsigned int i=0; i<num_symbols_ref; i++) {
        // generate a pair of QPSK symbol indices
        unsigned int sym = msequence_generate_symbol(ms, 2);

        // modulate into reference symbols
        syms_ref[i] = std::complex<float>(sym & 1 ? s2 : -s2, sym & 2 ? s2 : -s2);
    }
    msequence_destroy(ms);
}

sframe::~sframe()
{
    // free buffers
    delete [] syms_ref;
    delete [] syms_payload;
    delete [] syms_frame;
    delete [] buf_slot;

    // destroy mod/fec object
    qpacketmodem_destroy(mod);
}

void sframe::print(FILE * _fid)
{
    fprintf(_fid,"sframe: payload=%u bytes, syms:[%u guard, %u ref, %u frame, %u guard], frame=%u samples\n",
            payload_len,
            num_symbols_guard,
            num_symbols_ref,
            num_symbols_frame,
            num_symbols_guard,
            num_samples_slot);
}

