/*

  FFT wrapper object

  Copyright 2016
  Niels A. Moseley

  License: GPLv2

*/

#ifndef fft_h
#define fft_h

#include <math.h>
#include <vector>
#include "virtualmachine.h"
#include "kiss_fft.h"

/** 256-point FFT wrapper for kiss FFT */
class fft
{
public:
    fft();
    ~fft();

    /** process 256 complex-valued samples and calculate 256-point windowed fft */
    void process256(const VirtualMachine::ring_buffer_data_t *inbuffer,
                 VirtualMachine::ring_buffer_data_t *outbuffer);

    enum windowType {WIN_NONE, WIN_HAMMING, WIN_HANN, WIN_BLACKMAN, WIN_FLATTOP};

    /** setup the window for pre-weighting the time-domain samples */
    void setWindow(windowType wintype);

    /** return the currently selected window type */
    windowType getWindow() const
    {
        return m_winType;
    }

    enum mode_t
    {
        MODE_NORMAL,
        MODE_IQ
    };

    void setMode(mode_t mode)
    {
        m_mode = mode;
    }

    mode_t getMode() const
    {
        return m_mode;
    }
protected:
    std::vector<VirtualMachine::ring_buffer_data_t> m_data;
    std::vector<VirtualMachine::ring_buffer_data_t> m_result;
    std::vector<float> m_window;
    kiss_fft_cfg m_config;

    mode_t m_mode;

    windowType m_winType;
    float      m_avgConstant;
};

#endif
