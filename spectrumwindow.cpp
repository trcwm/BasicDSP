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

    // setup spectrum display
    m_spectrum = new SpectrumWidget(this);
    ui->mainLayout->addWidget(m_spectrum);

    //populate window options
    ui->windowTypeBox->addItem("None", 0);
    ui->windowTypeBox->addItem("Hann", 1);
    ui->windowTypeBox->addItem("Hamming", 2);
    ui->windowTypeBox->addItem("Blackman", 3);
    ui->windowTypeBox->addItem("Flattop", 4);

    fft::windowType wt = m_spectrum->getWindow();
    uint32_t idx = 0;
    switch(wt)
    {
    case fft::WIN_NONE:
        idx = 0;
        break;
    case fft::WIN_HANN:
        idx = 1;
        break;
    case fft::WIN_HAMMING:
        idx = 2;
        break;
    case fft::WIN_BLACKMAN:
        idx = 3;
        break;
    case fft::WIN_FLATTOP:
        idx = 4;
        break;
    default:
        break;
    }
    ui->windowTypeBox->setCurrentIndex(idx);

    // populate smoothing options
    ui->smoothingBox->addItem("None",0);
    ui->smoothingBox->addItem("10 frames",1);
    ui->smoothingBox->addItem("20 frames",2);
    ui->smoothingBox->addItem("50 frames",3);
    ui->smoothingBox->addItem("100 frames",4);
    ui->smoothingBox->setCurrentIndex(0);

    ui->modeBox->addItem("2 channel",0);
    ui->modeBox->addItem("IQ mode",1);
}

SpectrumWindow::~SpectrumWindow()
{
    delete ui;
}

void SpectrumWindow::setSampleRate(float rate)
{
    m_spectrum->setSampleRate(rate);
}

void SpectrumWindow::submit256Samples(const VirtualMachine::ring_buffer_data_t *samples)
{
    m_spectrum->submit256Samples(samples);
}

void SpectrumWindow::on_windowTypeBox_activated(int index)
{
    QVariant data = ui->windowTypeBox->itemData(index);
    if (!data.isNull())
    {
        switch(data.toInt())
        {
        case 0:
            m_spectrum->setWindow(fft::WIN_NONE);
            break;
        case 1:
            m_spectrum->setWindow(fft::WIN_HANN);
            break;
        case 2:
            m_spectrum->setWindow(fft::WIN_HAMMING);
            break;
        case 3:
            m_spectrum->setWindow(fft::WIN_BLACKMAN);
            break;
        case 4:
            m_spectrum->setWindow(fft::WIN_FLATTOP);
            break;
        }
    }
}

void SpectrumWindow::on_smoothingBox_activated(int index)
{
    m_spectrum->setSmoothingLevel(index);
}

void SpectrumWindow::on_modeBox_activated(int index)
{
    switch(index)
    {
    default:
    case 0: // regular 2-channel spectrum mode
        m_spectrum->setMode(fft::MODE_NORMAL);
        break;
    case 1: // IQ mode
        m_spectrum->setMode(fft::MODE_IQ);
        break;
    }
}
