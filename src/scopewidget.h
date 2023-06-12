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

    /** enable or disable the state */
    void setTriggerState(bool enabled);

    /** set to which channel the scope will trigger */
    void setTriggerChannel(uint32_t channelID)
    {
        m_trigChannel = channelID;
    }

    /** set the trigger level */
    void setTriggerLevel(float level)
    {
        m_trigLevel = level;
    }

protected:
    void paintEvent(QPaintEvent *event);

    int32_t y2pix(float yvalue);
    int32_t x2pix(float xvalue);

    std::vector<VirtualMachine::ring_buffer_data_t>  m_trigStorage;
    std::vector<VirtualMachine::ring_buffer_data_t>  m_signal;

    float       m_timespan;
    float       m_ymin,m_ymax;

    // trigger related variables
    bool        m_trigSearch;
    bool        m_trigEnabled;
    float       m_trigLevel;
    float       m_lastSample;
    uint32_t    m_trigChannel;
    uint32_t    m_samplesStored;

    bool        m_forceAxisRedraw;
    QImage      *m_bkbuffer;
};


#endif
