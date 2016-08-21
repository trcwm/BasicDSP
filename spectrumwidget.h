/*

  Spectrum display widget

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#ifndef spectrumwidget_h
#define spectrumwidget_h

#include <stdint.h>
#include <vector>
#include <QWidget>
#include <QImage>
#include "fft.h"
#include "virtualmachine.h"

class SpectrumWidget : public QWidget
{
    Q_OBJECT
public:
    SpectrumWidget(QWidget *parent);

    /** submit 256 time-domain samples */
    void submit256Samples(const VirtualMachine::ring_buffer_data_t *samples);

    /** set the window type for the FFT */
    void setWindow(fft::windowType type)
    {
        m_fft.setWindow(type);
    }

    /** get the current window type */
    fft::windowType getWindow() const
    {
        return m_fft.getWindow();
    }

    /** set the smoothing level */
    void setSmoothingLevel(uint32_t level);

    /** get the current smoothing level */
    uint32_t getSmoothingLevel() const
    {
        return m_smoothingLevel;
    }

    /** set the sample rate of the submitted data
        to generate the correct frequency axis */
    void setSampleRate(float rate)
    {
        m_sampleRate = rate;
        m_forceAxisRedraw = true;
    }

    /** set the mode of the FFT to normal
        (2-channel mode) or complex mode */
    void setMode(fft::mode_t mode)
    {
        m_fft.setMode(mode);
        m_forceAxisRedraw = true;
    }

protected:
    void paintEvent(QPaintEvent *event);

    int32_t db2pix(float db);
    int32_t x2pix(float xvalue);

    std::vector<float> m_dbData;
    std::vector<float> m_smoothed;
    std::vector<VirtualMachine::ring_buffer_data_t>  m_fftsig;

    float m_dbmin,m_dbmax;
    float m_fmin,m_fmax;
    float m_avgConstant;
    float m_sampleRate;
    bool  m_forceAxisRedraw;
    uint32_t m_smoothingLevel;

    QImage *m_bkbuffer;
    fft    m_fft;
};


#endif
