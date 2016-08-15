#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFile>
#include <QSettings>

#include "codeeditor.h"
#include "virtualmachine.h"
#include "namedslider.h"
#include "vumeter.h"
#include "spectrumwindow.h"
#include "scopewindow.h"
#include "fft.h"

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
    void on_actionSoundcard_triggered();

    void on_scopeButton_clicked();

    void on_actionFont_triggered();

    void on_pushButton_clicked();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

    void on_action_Open_triggered();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void readSettings();
    void writeSettings();

    /** save the script file given by m_filepath.
        if m_filepath is empty, prompt the user
        for a filename.
        write the script file
        if writing the file fails, prompt
        the user for a different filename.
        if the user cancels, return false.
        else retry to save the file with the
        new filename and return true if ok.
        else reprompt - ad nauseam */
    bool save();

    /** write the script to a file.
        returns true if succesful
    */
    bool saveScriptFile(const QString &filePath);

    /** write the script to a file.
        returns true if succesful
    */
    bool loadScriptFile(const QString &filePath);

    /** prompt the user for a filename */
    QString askForFilename();

    /** update the window title with the latest
        filename information.
    */
    void updateBasicDSPWindowTitle();

    Ui::MainWindow *ui;

    CodeEditor *m_sourceEditor;

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

    QSettings m_settings;
    QString   m_filepath;
    QString   m_lastDirectory;
};

#endif // MAINWINDOW_H
