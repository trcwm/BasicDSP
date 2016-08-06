/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#include "fft.h"

#define fft_size 256

fft::fft()
{
    const float pi2 = 2.0f*3.1415927f;
    const float Nm1 = fft_size-1;

    // setup forward FFT
    m_config = kiss_fft_alloc(fft_size,0,NULL,NULL);
    m_data.resize(fft_size);

    // setup hamming window
    m_window.resize(fft_size);
    for(uint32_t i=0; i<fft_size; i++)
    {
        //m_window[i] = (0.54f + 0.46f*cos(pi2*(float)i/Nm1))/sqrt((float)fft_size);
        m_window[i] = 1.0f/fft_size;
    }
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

    for(uint32_t i=0; i<fft_size; i++)
    {
        m_data[i].s1 = inbuffer[i].s1 * m_window[i];
        m_data[i].s2 = inbuffer[i].s2 * m_window[i];
    }

    kiss_fft(m_config, (const kiss_fft_cpx *)&m_data[0],
             (kiss_fft_cpx *)outbuffer);
}
