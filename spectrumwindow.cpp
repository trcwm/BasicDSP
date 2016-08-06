/*

  Spectrum display window

  Copyright 2006-2016
  Niels A. Moseley
  Pieter-Tjerk de Boer

  License: GPLv2

*/

#include "spectrumwindow.h"
#include "ui_spectrumwindow.h"

SpectrumWindow::SpectrumWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpectrumWindow)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    ui->setupUi(this);

    m_spectrum = new SpectrumWidget(this);
    ui->mainLayout->addWidget(m_spectrum);
}

SpectrumWindow::~SpectrumWindow()
{
    delete ui;
}

void SpectrumWindow::submit256Samples(const VirtualMachine::ring_buffer_data_t *samples)
{
    m_spectrum->submit256Samples(samples);
}
