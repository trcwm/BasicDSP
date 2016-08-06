/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#ifndef fft_h
#define fft_h

#include <vector>
#include "virtualmachine.h"
#include "kiss_fft.h"

/** 256-point FFT wrapper for kiss FFT */
class fft
{
public:
    fft();
    ~fft();

    void process256(const VirtualMachine::ring_buffer_data_t *inbuffer,
                 VirtualMachine::ring_buffer_data_t *outbuffer);

protected:
    std::vector<VirtualMachine::ring_buffer_data_t> m_data;
    std::vector<float> m_window;
    kiss_fft_cfg m_config;
};

#endif
