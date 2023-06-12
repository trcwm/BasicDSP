/*

  Scope widget

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/


#include <QPainter>
#include "scopewidget.h"

ScopeWidget::ScopeWidget(QWidget *parent)
    : QWidget(parent),
      m_bkbuffer(0),
      m_forceAxisRedraw(false)
{
    m_ymax = 1.1f;
    m_ymin = -1.1f;
    m_timespan = 100.0e-3f;

    m_trigEnabled = true;
    m_trigLevel = 0.0f;
    m_trigChannel = 0;
    m_trigSearch = false;
    m_samplesStored = 0;

    setTriggerState(true);

    m_trigStorage.resize(256);
    m_signal.resize(256);

    setMinimumSize(300,200);
}

void ScopeWidget::setTriggerState(bool enabled)
{
    if (enabled)
    {
        m_trigSearch = true;
        m_samplesStored = 0;
        m_lastSample = 0.0f;
    }
    else
    {
        m_trigSearch = false;
    }
    m_trigEnabled = enabled;
}

void ScopeWidget::submit256Samples(VirtualMachine::ring_buffer_data_t *buffer)
{
    // for triggering, we have to break the 256-sample boundaries
    // therefore, we must keep track of whether we're searching
    // for a trigger level crossing or whether we've just seen
    // this happen and are storing data.

    uint32_t idx = 0;   // search/read index
    if (m_trigEnabled)
    {
        // search for trigger, or store the data
        // if we've seen a trigger event
        while(idx < 256)
        {
            if (m_trigSearch)
            {
                // search for trigger
                float v;
                if (m_trigChannel == 0)
                {
                    v = buffer[idx].s1;
                }
                else
                {
                    v = buffer[idx].s2;
                }

                if ((v > m_trigLevel) && (m_lastSample < m_trigLevel))
                {
                    m_trigSearch = false;
                    m_trigStorage[m_samplesStored++] = buffer[idx];
                }
                idx++;
                m_lastSample = v;
            }
            else
            {
                // store the samples if there is still room
                if (m_samplesStored < 256)
                {
                    m_trigStorage[m_samplesStored++] = buffer[idx++];
                }
                else
                {
                    // we've filled the trigger storage
                    // so we can copy it to the actual display buffer
                    // and start a new search for the next trigger event
                    memcpy(&m_signal[0], &m_trigStorage[0], sizeof(VirtualMachine::ring_buffer_data_t)*256);
                    m_trigSearch = true;
                    m_samplesStored = 0;
                }
            }
        }
    }
    else
    {
        // no trigger, copy input directly
        memcpy(&m_signal[0], buffer, sizeof(VirtualMachine::ring_buffer_data_t)*256);
        m_trigSearch = false;
    }
}

void ScopeWidget::setSampleRate(float rate)
{
    m_timespan = 256.0f/rate;
    m_forceAxisRedraw = true;
}

int32_t ScopeWidget::y2pix(float yvalue)
{
    return static_cast<int32_t>((m_ymax - yvalue) / (m_ymax-m_ymin) * height());
}

int32_t ScopeWidget::x2pix(float xvalue)
{
    return static_cast<int32_t>(xvalue/256.0f*width());
}

void ScopeWidget::paintEvent(QPaintEvent *event)
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

        // draw the horizontal divisions
        QString string;
        QFontMetrics fm(font());
        uint32_t fontHeight = fm.height();

        /*
        float y_start = floor(m_ymin / 10.0f)*10.0f;
        float y_end   = floor(m_ymax / 10.0f)*10.0f;
        float y_step  = (y_end-y_start) / 10.0f;
        */

        float y_start = -1.1f;
        float y_end   = 1.1f;
        float y_step  = 0.2f;

        for(float y=y_start; y<=y_end; y+=y_step)
        {
            int32_t ypos = y2pix(y);
            bpainter.setPen(Qt::gray);
            bpainter.drawLine(0, ypos, width()-1, ypos);
            string = QString("%1").arg(y,2,'f',1);
            uint32_t fontWidth  = fm.horizontalAdvance(string)+2;
            QRect textRect(1,ypos-fontHeight/2,fontWidth,fontHeight);
            bpainter.fillRect(textRect,Qt::black);
            bpainter.setPen(Qt::white);
            bpainter.drawText(textRect,Qt::AlignCenter, string);
        }

        // draw the x-axis
        const float steps[] = {1.0e-3f, 2.0e-3f, 5.0e-3f, 10.0e-3f, 20.0e-3f,
                               50.0e-3f, 100.0e-3f,
                               200.0e-3f, 500e-3f, 1.0f, 2.0f, 0.0f};
        uint32_t idx = 0;
        uint32_t labelWidth  = fm.horizontalAdvance("XXXXXXXX");
        int maxLabels = width()/labelWidth;
        while(static_cast<int32_t>(m_timespan/steps[idx]) > maxLabels && steps[idx+1]>0)
            idx++;

        float step = steps[idx];

        int32_t start = 0;
        QRect textRect;
        for(float i=start; i<=m_timespan; i+=step)
        {
            int32_t x = static_cast<int32_t>(0.5f+i/m_timespan*width());
            bpainter.setPen(Qt::gray);
            bpainter.drawLine(x, 0, x, height()-1);

            bpainter.setPen(Qt::white);
            if (step>=1.0f)
            {
                string = QString("%1 s").arg(i,3,'d',0);
            }
            else
            {
                string = QString("%1 ms").arg((double)i*1000.0f,3,'d',0);
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
    painter.setPen(Qt::green);

    size_t N = m_signal.size();
    int32_t ypos_old = y2pix(m_signal[0].s1);
    int32_t xpos_old = x2pix(0);
    for(size_t i=1; i<N; i++)
    {
        int32_t ypos = y2pix(m_signal[i].s1);
        int32_t xpos = x2pix(i);
        painter.drawLine(xpos_old,ypos_old,xpos,ypos);
        ypos_old = ypos;
        xpos_old = xpos;
    }

    painter.setPen(Qt::yellow);
    N = m_signal.size();
    ypos_old = y2pix(m_signal[0].s2);
    xpos_old = x2pix(0);
    for(size_t i=1; i<N; i++)
    {
        int32_t ypos = y2pix(m_signal[i].s2);
        int32_t xpos = x2pix(i);
        painter.drawLine(xpos_old,ypos_old,xpos,ypos);
        ypos_old = ypos;
        xpos_old = xpos;
    }
}

