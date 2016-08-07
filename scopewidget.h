/*

  Scope widget

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#ifndef scopewidget_h
#define scopewidget_h

#include <stdint.h>
#include <vector>
#include <QWidget>
#include <QImage>
#include "virtualmachine.h"

class ScopeWidget : public QWidget
{
    Q_OBJECT
public:
    ScopeWidget(QWidget *parent);

    void submit256Samples(VirtualMachine::ring_buffer_data_t *buffer);

    /** set sample rate for correct x-axis scaling */
    void setSampleRate(float rate);

protected:
    void paintEvent(QPaintEvent *event);

    int32_t y2pix(float yvalue);
    int32_t x2pix(float xvalue);

    std::vector<VirtualMachine::ring_buffer_data_t>  m_signal;

    float m_timespan;
    float m_ymin,m_ymax;
    bool m_forceAxisRedraw;
    QImage *m_bkbuffer;
};


#endif
