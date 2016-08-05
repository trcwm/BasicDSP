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

class ScopeWidget : public QWidget
{
    Q_OBJECT
public:
    ScopeWidget(QWidget *parent);

    void submit256Samples(float *buffer);

protected:
    void paintEvent(QPaintEvent *event);

    int32_t y2pix(float yvalue);

    std::vector<float>  m_signal;
    std::vector<float>  m_sbuffer;
    uint32_t            m_sbufferIdx;

    float m_timespan;
    float m_ymin,m_ymax;

    QImage *m_bkbuffer;
};


#endif
