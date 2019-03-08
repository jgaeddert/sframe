// slot frame generator

#include "sframesync.hh"

sframesync::sframesync(unsigned int _payload_len) :
    sframe(_payload_len)
{
}

sframesync::~sframesync()
{
}

void * sframesync::receive(std::complex<float> * _buf)
{
    return 0;
}
