/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#include "fft.h"

#define fft_size 256

fft::fft()
    : m_mode(MODE_NORMAL)
{
    // setup forward FFT
    m_config = kiss_fft_alloc(fft_size,0,NULL,NULL);
    m_data.resize(fft_size);
    m_result.resize(fft_size);

    // setup hamming & hann windows
    m_window.resize(fft_size);
    setWindow(WIN_FLATTOP);
}

fft::~fft()
{
    kiss_fft_free(m_config);
}

void fft::setWindow(windowType wintype)
{
    const float pi = 3.1415927f;
    const float pi2 = 2.0f*3.1415927f;
    const float pi4 = 4.0f*3.1415927f;
    const float Nm1 = fft_size-1;

    double sum = 0.0f;
    for(uint32_t i=0; i<fft_size; i++)
    {
        switch(wintype)
        {
        case WIN_NONE:
            m_window[i] = 1.0f;
            break;
        case WIN_HANN:
            m_window[i] = 0.50f - 0.50f*cos(pi2*(float)i/Nm1);
            break;
        case WIN_HAMMING:
            m_window[i] = 0.54f - 0.46f*cos(pi2*(float)i/Nm1);
            break;
        case WIN_BLACKMAN:
            m_window[i] = 0.42659f - 0.49656f*cos(pi2*(float)i/Nm1)
                                   + 0.076849f*cos(pi4*(float)i/Nm1);
            break;
        case WIN_FLATTOP:
            m_window[i] = 1.0f;
            m_window[i]-= 1.93f*cos(2.0f*pi*(float)i/Nm1);
            m_window[i]+= 1.29f*cos(4.0f*pi*(float)i/Nm1);
            m_window[i]-= 0.388f*cos(6.0f*pi*(float)i/Nm1);
            m_window[i]+= 0.028f*cos(8.0f*pi*(float)i/Nm1);
            break;
        }
        sum+=m_window[i];
    }

    // normalize the window so that a sine wave without leakage
    // is always at 0 dB
    //
    // fft normalization factor is sqrt(2.0f)/fft_size
    // window normalization factor is 256/sum;

    for(uint32_t i=0; i<fft_size; i++)
    {
        m_window[i] *= 256.0f/(float)fft_size/sum;
    }
    m_winType = wintype;
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

    switch(m_mode)
    {
    default:
    case MODE_NORMAL:   // normal 2-channel mode
        kiss_fft(m_config, (const kiss_fft_cpx *)&m_data[0],
                 (kiss_fft_cpx *)&m_result[0]);

        for(uint32_t i=1; i<fft_size/2; i++)
        {
            outbuffer[i].s1 = (m_result[i].s1 + m_result[fft_size-i].s1);
            outbuffer[i].s2 = (m_result[i].s2 - m_result[fft_size-i].s2);
        }
        outbuffer[0].s1 = (m_result[0].s1 + m_result[0].s1)/4.0f;
        outbuffer[0].s2 = (m_result[0].s2 - m_result[0].s2)/4.0f;

        for(uint32_t i=1; i<fft_size/2; i++)
        {
            outbuffer[i+(fft_size/2)].s1 = (m_result[i].s2 + m_result[fft_size-i].s2);
            outbuffer[i+(fft_size/2)].s2 = (-m_result[i].s1 + m_result[fft_size-i].s1);
        }
        outbuffer[(fft_size/2)].s1 = (m_result[0].s2 + m_result[0].s2)/4.0f;
        outbuffer[(fft_size/2)].s2 = (-m_result[0].s1 + m_result[0].s1)/4.0f;
        break;
    case MODE_IQ:       // no change needed!
        kiss_fft(m_config, (const kiss_fft_cpx *)&m_data[0],
                 (kiss_fft_cpx *)outbuffer);
        break;
    }
}


