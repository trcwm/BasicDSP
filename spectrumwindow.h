/*

  Spectrum display window

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#ifndef SPECTRUMWINDOW_H
#define SPECTRUMWINDOW_H

#include <QDialog>
#include "spectrumwidget.h"
#include "virtualmachine.h"

namespace Ui {
class SpectrumWindow;
}

class SpectrumWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SpectrumWindow(QWidget *parent = 0);
    ~SpectrumWindow();

    /** submit 256 FFT'd samples */
    void submit256Samples(const VirtualMachine::ring_buffer_data_t *samples);

    /** set the sample rate for correct frequency axis scaling */
    void setSampleRate(float rate);

private slots:
    void on_windowTypeBox_activated(int index);

    void on_smoothingBox_activated(int index);

    void on_modeBox_activated(int index);

private:
    Ui::SpectrumWindow *ui;
    SpectrumWidget      *m_spectrum;    
};

#endif // SPECTRUMWINDOW_H
