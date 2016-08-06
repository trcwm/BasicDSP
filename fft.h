/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#ifndef fft_h
#define fft_h

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
    kiss_fft_cfg m_config;
};

#endif
