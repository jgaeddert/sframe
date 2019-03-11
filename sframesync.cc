// slot frame generator

#include "sframesync.hh"

sframesync::sframesync(unsigned int _payload_len) :
    sframe(_payload_len),
    detector(this)
{
    payload_rec = new unsigned char[_payload_len];

    // create objects for frame recovery
    mixer = nco_crcf_create(LIQUID_NCO);
    npfb  = 16;
    mf    = firpfb_crcf_create_rnyquist(LIQUID_FIRFILT_ARKAISER, npfb, k, m, beta);
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

void * sframesync::receive(const std::complex<float> * _buf)
{
    // run detector and evaluate results
    sdetect::results results = detector.execute(_buf);
    
    printf("results:\n");
    printf("  rms      = %12.8f\n", results.rms);
    printf("  rxy      = %12.8f\n", results.rxy);
    printf("  tau_hat  = %12.8f\n", results.tau_hat);
    printf("  dphi_hat = %12.8f\n", results.dphi_hat);
    printf("  phi_hat  = %12.8f\n", results.phi_hat);

    // set mixer frequency appropriately; ignore phase offset
    nco_crcf_set_frequency(mixer, results.dphi_hat);

    // reset matched filter and compute values for timing recovery
    firpfb_crcf_reset(mf);
    int          dt        = (int)std::floor(results.tau_hat);
    unsigned int pfb_index = (unsigned int)std::round((results.tau_hat - (float)dt) * npfb);
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
    int crc_pass =
    qpacketmodem_decode_soft(mod, syms_payload, payload_rec);
    printf("crc : %s\n", crc_pass ? "pass!" : "FAIL!");

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

    return NULL;
}
