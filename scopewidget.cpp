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
    : QWidget(parent), m_bkbuffer(0)
{
    m_ymax = 1.1f;
    m_ymin = -1.1f;
    m_timespan = 100.0e-3f;

    m_signal.resize(256);
}

void ScopeWidget::submit256Samples(VirtualMachine::ring_buffer_data_t *buffer)
{
    memcpy(&m_signal[0], buffer, sizeof(VirtualMachine::ring_buffer_data_t)*256);
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
            || (m_bkbuffer->height() != height()))
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
            uint32_t fontWidth  = fm.width(string)+2;
            QRect textRect(1,ypos-fontHeight/2,fontWidth,fontHeight);
            bpainter.fillRect(textRect,Qt::black);
            bpainter.setPen(Qt::white);
            bpainter.drawText(textRect,Qt::AlignCenter, string);
        }

        // draw the x-axis
        const float steps[] = {1.0e-3, 2.0e-3, 5.0e-3, 10.0e-3, 20.0e-3,
                               50.0e-3, 100.0e-3,
                               200.0e-3, 500e-3, 1, 2, 0};
        uint32_t idx = 0;
        uint32_t labelWidth  = fm.width("XXXXXXXX");
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
            int32_t txtWidth  = fm.width(string)+2;
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

