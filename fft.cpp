/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#include "fft.h"

fft::fft()
{
    m_config = kiss_fft_alloc(256,0,NULL,NULL);
}

fft::~fft()
{
    kiss_fft_free(m_config);
}

void fft::process256(const VirtualMachine::ring_buffer_data_t *inbuffer,
             VirtualMachine::ring_buffer_data_t *outbuffer)
{
    if (sizeof(kiss_fft_cpx) != sizeof(VirtualMachine::ring_buffer_data_t))
    {
        // we have a compiler problem!
        // both datatypes should contain two floats and
        // therefore be of the same size!
        throw std::runtime_error("fft::process sizes of datatypes don't match!");
    }

    kiss_fft(m_config, (const kiss_fft_cpx *)inbuffer,
             (kiss_fft_cpx *)outbuffer);
}
