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

    void submit256Samples(VirtualMachine::ring_buffer_data_t *samples);

private:
    Ui::SpectrumWindow *ui;
    SpectrumWidget      *m_spectrum;    
};

#endif // SPECTRUMWINDOW_H
