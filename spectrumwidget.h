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

class SpectrumWidget : public QWidget
{
    Q_OBJECT
public:
    SpectrumWidget(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event);

    int32_t db2pix(float db);

    std::vector<float>  m_sig1;
    std::vector<float>  m_sig2;

    float m_dbmin,m_dbmax;
    float m_fmin,m_fmax;

    QImage *m_bkbuffer;
};


#endif
