#ifndef SOUNDCARDDIALOG_H
#define SOUNDCARDDIALOG_H

#include <QDialog>
#include "portaudio.h"

namespace Ui {
class SoundcardDialog;
}

class SoundcardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SoundcardDialog(QWidget *parent = 0);
    ~SoundcardDialog();

    void setInputSource(PaDeviceIndex device);
    void setOutputSource(PaDeviceIndex device);
    void setSamplerate(float rate);

    PaDeviceIndex getInputSource();
    PaDeviceIndex getOutputSource();
    float         getSamplerate();


private slots:
    void on_inputComboBox_currentIndexChanged(int index);

    void on_outputComboBox_currentIndexChanged(int index);

private:
    void checkSupport();
    Ui::SoundcardDialog *ui;
};

#endif // SOUNDCARDDIALOG_H
