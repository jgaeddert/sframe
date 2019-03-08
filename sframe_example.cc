#include "sframegen.hh"
#include "sframesync.hh"

int main() {
    // options
    unsigned int payload_len = 256;

    // create objects
    sframegen  gen  = sframegen (payload_len);
    sframesync sync = sframesync(payload_len);

    gen.print();
    sync.print();

    return 0;
}

