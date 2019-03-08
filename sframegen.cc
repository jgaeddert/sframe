// slot frame generator

#include <string.h>
#include "sframegen.hh"

sframegen::sframegen(unsigned int _payload_len) :
    sframe(_payload_len)
{
}

sframegen::~sframegen()
{
}

const std::complex<float> * sframegen::generate(unsigned char * _payload)
{
    // reset buffer
    memset(buf_slot, 0x00, num_samples_slot*sizeof(std::complex<float>));

    return buf_slot;
}
