#ifndef SPECTRUMWINDOW_H
#define SPECTRUMWINDOW_H

#include <QDialog>
#include "spectrumwidget.h"

namespace Ui {
class SpectrumWindow;
}

class SpectrumWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SpectrumWindow(QWidget *parent = 0);
    ~SpectrumWindow();

private:
    Ui::SpectrumWindow *ui;
    SpectrumWidget      *m_spectrum;
};

#endif // SPECTRUMWINDOW_H
