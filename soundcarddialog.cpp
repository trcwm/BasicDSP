#include "soundcarddialog.h"
#include "ui_soundcarddialog.h"
#include "portaudio_helper.h"

SoundcardDialog::SoundcardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SoundcardDialog)
{
    ui->setupUi(this);

    // populate comboboxes
    PaDeviceIndex count = Pa_GetDeviceCount();
    for(PaDeviceIndex idx=0; idx<count; idx++)
    {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(idx);
        QString deviceName = PA_Helper::getDeviceString(idx);
        if (info->maxInputChannels > 0)
        {
            ui->inputComboBox->addItem(deviceName, QVariant(idx));
        }
        if (info->maxOutputChannels > 0)
        {
            ui->outputComboBox->addItem(deviceName, QVariant(idx));
        }
    }
}

SoundcardDialog::~SoundcardDialog()
{
    delete ui;
}

PaDeviceIndex SoundcardDialog::getInputSource()
{
    int index = ui->inputComboBox->currentIndex();
    if (index == -1)
    {
        // no item was selected, so we return the
        // default input device
        return Pa_GetDefaultInputDevice();
    }
    else
    {
        bool ok;
        PaDeviceIndex idx = ui->inputComboBox->itemData(index).toInt(&ok);
        if (!ok)
        {
            // conversion to int not succesfull
            // return the default input device .. :(
            return Pa_GetDefaultInputDevice();
        }
        return idx;
    }
}

PaDeviceIndex SoundcardDialog::getOutputSource()
{
    int index = ui->outputComboBox->currentIndex();
    if (index == -1)
    {
        // no item was selected, so we return the
        // default input device
        return Pa_GetDefaultOutputDevice();
    }
    else
    {
        bool ok;
        PaDeviceIndex idx = ui->outputComboBox->itemData(index).toInt(&ok);
        if (!ok)
        {
            // conversion to int not succesfull
            // return the default input device .. :(
            return Pa_GetDefaultOutputDevice();
        }
        return idx;
    }
}

void SoundcardDialog::setInputSource(PaDeviceIndex device)
{
    int N = ui->inputComboBox->count();
    for(int i=0; i<N; i++)
    {
        bool ok;
        PaDeviceIndex idx = ui->inputComboBox->itemData(i).toInt(&ok);
        if ((ok) && (idx == device))
        {
            ui->inputComboBox->setCurrentIndex(i);
            return;
        }
    }
}

void SoundcardDialog::setOutputSource(PaDeviceIndex device)
{
    int N = ui->outputComboBox->count();
    for(int i=0; i<N; i++)
    {
        bool ok;
        PaDeviceIndex idx = ui->outputComboBox->itemData(i).toInt(&ok);
        if ((ok) && (idx == device))
        {
            ui->outputComboBox->setCurrentIndex(i);
            return;
        }
    }
}

void SoundcardDialog::setSamplerate(float rate)
{
    ui->sampleRate->setText(QString("%1").arg(rate,5,'d',0));
}

float SoundcardDialog::getSamplerate()
{
    bool ok;
    float rate = ui->sampleRate->text().toFloat(&ok);
    if (!ok)
    {
        return 44100.0;
    }
    return rate;
}
