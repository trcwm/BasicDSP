#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "virtualmachine.h"
#include "namedslider.h"
#include "vumeter.h"

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

    void on_runButton_clicked();

private:
    Ui::MainWindow *ui;

    NamedSlider *m_slider1;
    NamedSlider *m_slider2;
    NamedSlider *m_slider3;
    NamedSlider *m_slider4;

    QTimer  *m_guiTimer;

    VUMeter *m_rightVUMeter;
    VUMeter *m_leftVUMeter;

    VirtualMachine *machine;
};

#endif // MAINWINDOW_H
