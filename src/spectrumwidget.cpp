/*

  Spectrum display widget

  Copyright 2006-2017
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#include <stdint.h>
#include <QPainter>
#include <QFontDatabase>
#include "spectrumwidget.h"

SpectrumWidget::SpectrumWidget(QWidget *parent)
    : QWidget(parent),
      m_bkbuffer(0),
      m_avgConstant(0.0f),
      m_smoothingLevel(0),
      m_forceAxisRedraw(true)
{
    m_dbmin = -65.0f;
    m_dbmax = 5.0f;
    m_fmin  = 0.0f;
    m_fmax  = 8000.0f;

    const QFont smallFont = QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont);
    setFont(smallFont);

    m_dbData.resize(256);
    m_fftsig.resize(256);
    m_smoothed.resize(256);
}

/** Set the time constant for smoothing.
    This factor is applied once every FFT frame,
    so time is relative to the number of
    frames.

    Time constant (66%):
    level 0: no smoothing
    level 1: 10 frames
    level 2: 20 frames
    level 3: 50 frames
    level 4: 100 frames
*/

void SpectrumWidget::setSmoothingLevel(uint32_t level)
{
    switch(level)
    {
    default:
    case 0:
        m_avgConstant = 0.0f;
        break;
    case 1:
        m_avgConstant = pow(0.33f, 1.0f/10.0f);
        break;
    case 2:
        m_avgConstant = pow(0.33f, 1.0f/20.0f);
        break;
    case 3:
        m_avgConstant = pow(0.33f, 1.0f/50.0f);
        break;
    case 4:
        m_avgConstant = pow(0.33f, 1.0f/100.0f);
        break;
    }
    m_smoothingLevel = level;
}

void SpectrumWidget::submit256Samples(const VirtualMachine::ring_buffer_data_t *samples)
{
    m_fft.process256(samples, &m_fftsig[0]);
    for(uint32_t i=0; i<256; i++)
    {
        // add 1e-20f to stop log10 from producing NaNs.
        float mag = m_fftsig[i].s1*m_fftsig[i].s1 + m_fftsig[i].s2*m_fftsig[i].s2+1e-20f;
        m_smoothed[i] = mag + m_avgConstant*(m_smoothed[i]-mag);
        // check for NaNs
        if (m_smoothed[i] != m_smoothed[i])
        {
            m_smoothed[i] = 1e-20f;
        }
        m_dbData[i] = 10.0f*log10(m_smoothed[i]);
    }
}

void SpectrumWidget::paintEvent(QPaintEvent *event)
{
    (event);

    // create a new back buffer if the widget got
    // resized or if there isn't one
    if ((m_bkbuffer == 0) || (m_bkbuffer->width() != width())
            || (m_bkbuffer->height() != height())
            || m_forceAxisRedraw)
    {
        if (m_bkbuffer != 0)
            delete m_bkbuffer;

        m_bkbuffer = new QImage(rect().size(), QImage::Format_RGB32);
        QPainter bpainter(m_bkbuffer);

        bpainter.fillRect(rect(), Qt::black);

        // calculate the frequency axis span
        switch(m_fft.getMode())
        {
        case fft::MODE_NORMAL:
            m_fmin  = 0.0f;
            m_fmax  = m_sampleRate/2.0f;
            break;
        case fft::MODE_IQ:
            m_fmin  = -m_sampleRate/2.0f;
            m_fmax  = m_sampleRate/2.0f;
            break;
        }

        // draw the horizontal divisions
        QString string;
        QFontMetrics fm(font());
        uint32_t fontHeight = fm.height();

        float db_start = floor(m_dbmin / 10.0f)*10.0f;
        float db_end   = floor(m_dbmax / 10.0f)*10.0f;

        if (db_end-db_start > 10.0f)
        {
            for(float db=db_start; db<=db_end; db+=10.0f)
            {
                int32_t ypos = db2pix(db);
                bpainter.setPen(Qt::gray);
                bpainter.drawLine(0, ypos, width()-1, ypos);
                string = QString("%1dB").arg(db,3,'d',0);
                uint32_t fontWidth  = fm.horizontalAdvance(string)+2;
                QRect textRect(1,ypos-fontHeight/2,fontWidth,fontHeight);
                bpainter.fillRect(textRect,Qt::black);
                bpainter.setPen(Qt::white);
                bpainter.drawText(textRect,Qt::AlignCenter, string);
            }
        }

        // draw the x-axis
        const int32_t steps[] = {1,2,5,10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,0};
        uint32_t idx = 0;
        uint32_t labelWidth  = fm.horizontalAdvance("XXXXXXXX");
        int maxLabels = width()/labelWidth;
        while((m_fmax-m_fmin)/steps[idx] > maxLabels && steps[idx+1]>0)
            idx++;

        int32_t step = steps[idx];

        int32_t start = -step*(static_cast<int32_t>(-m_fmin/step+1000)-1000);
        QRect textRect;
        for(int32_t i=start; i<=m_fmax; i+=step)
        {
            int32_t x = static_cast<int32_t>(0.5f+(i-m_fmin)/(m_fmax-m_fmin)*width());
            bpainter.setPen(Qt::gray);
            bpainter.drawLine(x, 0, x, height()-1);

            bpainter.setPen(Qt::white);
            if (step>=1000)
            {
                string = QString("%1 kHz").arg(i/1000.0f,3,'d',0);
            }
            else
            {
                string = QString("%1 Hz").arg((double)i,3,'d',0);
            }
            int32_t txtWidth  = fm.horizontalAdvance(string)+2;
            if ((x-txtWidth/2.0f) < 0)
            {
                // adjust for off-screen label left side
                textRect = QRect(0,height()-fontHeight,txtWidth,fontHeight);
            }
            else if ((x+txtWidth/2.0f) > width())
            {
                // adjust for off-screen label right side
                textRect = QRect(width()-txtWidth,height()-fontHeight,txtWidth,fontHeight);
            }
            else
            {
                textRect = QRect(x-txtWidth/2,height()-fontHeight,txtWidth,fontHeight);
            }
            bpainter.fillRect(textRect, Qt::black);
            bpainter.drawText(textRect,Qt::AlignCenter, string);
        }
        m_forceAxisRedraw = false;
    }
    QPainter painter(this);
    painter.drawImage(rect(), *m_bkbuffer);

    int32_t ypos_old;
    int32_t xpos_old;
    switch(m_fft.getMode())
    {
    case fft::MODE_NORMAL:
        painter.setPen(Qt::yellow);
        ypos_old = db2pix(m_dbData[0]);
        xpos_old = 0;
        for(uint32_t i=1; i<128; i++)
        {
            int32_t ypos = db2pix(m_dbData[i]);
            int32_t xpos = x2pix(i);
            painter.drawLine(xpos_old, ypos_old, xpos, ypos);
            xpos_old = xpos;
            ypos_old = ypos;
        }
        painter.setPen(Qt::green);
        ypos_old = db2pix(m_dbData[128]);
        xpos_old = 0;
        for(uint32_t i=129; i<256; i++)
        {
            int32_t ypos = db2pix(m_dbData[i]);
            int32_t xpos = x2pix(i-128);
            painter.drawLine(xpos_old, ypos_old, xpos, ypos);
            xpos_old = xpos;
            ypos_old = ypos;
        }
        break;
    case fft::MODE_IQ:
        painter.setPen(Qt::cyan);
        ypos_old = db2pix(m_dbData[0]);
        xpos_old = x2pix(128);
        // first, the positive half
        // of the spectrum
        for(uint32_t i=1; i<128; i++)
        {
            int32_t ypos = db2pix(m_dbData[i]);
            int32_t xpos = x2pix(i+128);
            painter.drawLine(xpos_old, ypos_old, xpos, ypos);
            xpos_old = xpos;
            ypos_old = ypos;
        }
        // then, the negative half..
        ypos_old = db2pix(m_dbData[128]);
        xpos_old = x2pix(0);
        for(uint32_t i=1; i<128; i++)
        {
            int32_t ypos = db2pix(m_dbData[i+128]);
            int32_t xpos = x2pix(i);
            painter.drawLine(xpos_old, ypos_old, xpos, ypos);
            xpos_old = xpos;
            ypos_old = ypos;
        }
        // connect the two halves!
        int32_t ypos = db2pix(m_dbData[0]);
        int32_t xpos = x2pix(128);
        painter.drawLine(xpos_old, ypos_old, xpos, ypos);
    }
}

int32_t SpectrumWidget::db2pix(float db)
{
   return static_cast<int32_t>((m_dbmax - db) / (m_dbmax-m_dbmin) * height());
}

int32_t SpectrumWidget::x2pix(float xvalue)
{
    switch(m_fft.getMode())
    {
    default:
    case fft::MODE_NORMAL:
        return static_cast<int32_t>(xvalue/128.0f*width());
        break;
    case fft::MODE_IQ:
        return static_cast<int32_t>(xvalue/256.0f*width());
        break;
    }
}
