#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "virtualmachine.h"
#include "namedslider.h"
#include "vumeter.h"
#include "spectrumwindow.h"
#include "scopewindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();
    void on_GUITimer();
    void on_Slider1Changed(float value);
    void on_Slider2Changed(float value);
    void on_Slider3Changed(float value);
    void on_Slider4Changed(float value);
    void on_runButton_clicked();

    void on_SourceChanged();

    void on_stopButton_clicked();

private:
    Ui::MainWindow *ui;

    NamedSlider *m_slider1;
    NamedSlider *m_slider2;
    NamedSlider *m_slider3;
    NamedSlider *m_slider4;

    QTimer  *m_guiTimer;

    VUMeter *m_rightVUMeter;
    VUMeter *m_leftVUMeter;

    VirtualMachine *m_machine;

    SpectrumWindow *m_spectrum;
    ScopeWindow    *m_scope;
};

#endif // MAINWINDOW_H
