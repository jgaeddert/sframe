// slot frame generator

#include "sframesync.hh"

sframesync::sframesync(unsigned int _payload_len) :
    sframe(_payload_len),
    detector(this),
    rxy_threshold(0) // attempt to decode all frames
{
    payload_rec = new unsigned char[_payload_len];

    // create objects for frame recovery
    mixer = nco_crcf_create(LIQUID_NCO);
    npfb  = 16;
    mf    = firpfb_crcf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, npfb, k, m, beta);
    firpfb_crcf_set_scale(mf, 1.0f/(float)k);

    sync  = qpilotsync_create(num_symbols_payload, pilot_spacing);
}

sframesync::~sframesync()
{
    delete [] payload_rec;

    // destroy allocated objects
    nco_crcf_destroy   (mixer);
    firpfb_crcf_destroy(mf);
    qpilotsync_destroy (sync);
}

sframesync::results sframesync::receive(const std::complex<float> * _buf)
{
    // run detector and evaluate results
    sdetect::results r = detector.execute(_buf);

    // assemble results object and return if threshold is not exceeded
    sframesync::results results(r);
    if (results.rxy < rxy_threshold)
        return results;
    
    // set mixer frequency appropriately; ignore phase offset
    nco_crcf_set_frequency(mixer, r.dphi_hat);

    // reset matched filter and compute values for timing recovery
    firpfb_crcf_reset(mf);
    int          dt        = (int)std::floor(r.tau_hat);
    unsigned int pfb_index = (unsigned int)std::round((r.tau_hat - (float)dt) * npfb);
    if (pfb_index >= npfb) { dt++; pfb_index -= npfb; }
    // assert( dt isn't too large )
    unsigned int offset = k*( m + num_symbols_guard + num_symbols_ref ) + dt;
    //offset = std::min(num_samples_slot, offset);

    // mix down and push through matched filter
    unsigned int i                   = 0; // input sample counter
    unsigned int num_symbols_written = 0; // number of symbols written to buffer
    unsigned int n                   = 0; // samples within a symbol
    while (num_symbols_written < num_symbols_frame) {
        // mix input down
        std::complex<float> nco_in = i < num_samples_slot ? _buf[i] : 0;
        std::complex<float> nco_out;
        nco_crcf_mix_down(mixer, nco_in, &nco_out);
        nco_crcf_step    (mixer);
        i++;

        // push result into matched filter
        firpfb_crcf_push(mf, nco_out);

        if (i < offset)
            continue;

        // downsample, saving appropriate symbols to buffer
        n++;
        if ( n != k )
            continue;

        // save this symbol in buffer
        n = 0;  // reset sample counter
        firpfb_crcf_execute(mf, pfb_index, &syms_frame[num_symbols_written]);
        num_symbols_written++;
    }

    // recover payload symbols (fine carrier frequency/phase recovery)
    qpilotsync_execute(sync, syms_frame, syms_payload);

    // demodulate/decode payload
    results.crc_pass = qpacketmodem_decode_soft(mod, syms_payload, payload_rec);

    // populate results field
    results.payload     =  (const unsigned char *)payload_rec;
    results.payload_len =  payload_len;
    results.dphi_hat    += qpilotsync_get_dphi(sync) / (float)k;
    results.rssi        =  10*log10f( qpilotsync_get_gain(sync) );
    results.evm         =  20*log10f( qpacketmodem_get_demodulator_evm(mod) + 1e-6f );
    results.syms        =  (const std::complex<float>*) syms_payload;
    results.num_syms    =  num_symbols_payload;

#if 0
    // dump results
    FILE * fid = fopen("sframesync_frame.dat","w");
    for (unsigned int i=0; i<num_symbols_frame; i++)
        fprintf(fid,"%12.4e %12.4e\n", syms_frame[i].real(), syms_frame[i].imag());
    fclose(fid);

    fid = fopen("sframesync_payload.dat","w");
    for (unsigned int i=0; i<num_symbols_payload; i++)
        fprintf(fid,"%12.4e %12.4e\n", syms_payload[i].real(), syms_payload[i].imag());
    fclose(fid);
#endif

    return results;
}

//
// results object
//

sframesync::results::results() :
    payload(NULL), payload_len(0),
    crc_pass(false),
    rssi(0), rxy(0), tau_hat(0), dphi_hat(0), evm(0),
    syms(NULL), num_syms(0)
{
}

sframesync::results::results(sdetect::results & _r) :
    sframesync::results()
{
    rssi     = 20*log10f(_r.rms);
    rxy      = _r.rxy;
    tau_hat  = _r.tau_hat;
    dphi_hat = _r.dphi_hat;
}

void sframesync::results::print()
{
    printf("sframesync results:\n");
    printf("  rssi     = %12.6f dB\n",         rssi);
    printf("  rxy      = %12.6f\n",            rxy);
    printf("  tau_hat  = %12.6f samples\n",    tau_hat);
    printf("  dphi_hat = %12.6f rad/sample\n", dphi_hat);
    printf("  evm      = %12.6f dB\n",         evm);
    if (num_syms >= 4) {
        printf("  syms     = (%6.3f,%6.3f) (%6.3f,%6.3f) (%6.3f,%6.3f) (%6.3f,%6.3f)...\n",
                syms[0].real(), syms[0].imag(),
                syms[1].real(), syms[1].imag(),
                syms[2].real(), syms[2].imag(),
                syms[3].real(), syms[3].imag());
    }
    if (payload != NULL && payload_len >= 4) {
        printf("  payload  = [");
        for (unsigned int i=0; i<std::min(payload_len, 20U); i++)
            printf(" %.2x", payload[i]);
        printf("...]\n");
        printf("  payload  = %s\n", crc_pass ? "valid" : "INVALID");
    }
}

