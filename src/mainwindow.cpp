#include <QCoreApplication>
#include <QDebug>
#include <QFontDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>

#include "ui_mainwindow.h"

#include <sstream>
#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "asttovm.h"
#include "mainwindow.h"
#include "pa_ringbuffer.h"
#include "portaudio_helper.h"
#include "soundcarddialog.h"
#include "aboutdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settings("MoseleyInstruments","BasicDSP")
{
    ui->setupUi(this);

    /** set code editor */
    m_sourceEditor = new CodeEditor(this);
    ui->sourceEditorLayout->addWidget(m_sourceEditor);

    /** set source editor font */
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    m_sourceEditor->setFont(fixedFont);

    /** add four sliders programatically */
    m_slider1 = new NamedSlider("slider 1", this);
    ui->mainLayout->addWidget(m_slider1);
    m_slider2 = new NamedSlider("slider 2", this);
    ui->mainLayout->addWidget(m_slider2);
    m_slider3 = new NamedSlider("slider 3", this);
    ui->mainLayout->addWidget(m_slider3);
    m_slider4 = new NamedSlider("slider 4", this);
    ui->mainLayout->addWidget(m_slider4);

    connect(m_slider1, SIGNAL(valueChanged(float)), this, SLOT(on_Slider1Changed(float)));
    connect(m_slider2, SIGNAL(valueChanged(float)), this, SLOT(on_Slider2Changed(float)));
    connect(m_slider3, SIGNAL(valueChanged(float)), this, SLOT(on_Slider3Changed(float)));
    connect(m_slider4, SIGNAL(valueChanged(float)), this, SLOT(on_Slider4Changed(float)));

    /** add VU meters */
    m_leftVUMeter = new VUMeter("Left", this);
    ui->mainLayout->addWidget(m_leftVUMeter);
    m_rightVUMeter = new VUMeter("Right", this);
    ui->mainLayout->addWidget(m_rightVUMeter);

    /** connect the source selection buttons to their handler */
    connect(ui->inputAudioFile, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));
    connect(ui->inputQuadSine, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));
    connect(ui->inputImpulse, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));
    connect(ui->inputSineWave, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));
    connect(ui->inputWhiteNoise, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));
    connect(ui->inputSoundcard, SIGNAL(clicked(bool)), this, SLOT(on_SourceChanged()));

    /** create the virtual machine */
    m_machine = new VirtualMachine(this);

    /** create a GUI timer to update VU etc */
    m_guiTimer = new QTimer(this);
    connect(m_guiTimer, SIGNAL(timeout()), this, SLOT(on_GUITimer()));
    m_guiTimer->start(100);

    /** create a spectrum window */
    m_spectrum = new SpectrumWindow(this);


    /** create a scope window */
    m_scope = new ScopeWindow(this);

    connect(m_scope, SIGNAL(channelChanged(uint32_t)), this, SLOT(scopeChannelChanged(uint32_t)));
    connect(m_spectrum, SIGNAL(channelChanged(uint32_t)), this, SLOT(spectrumChannelChanged(uint32_t)));

    /** get the progam setting */
    readSettings();
}

MainWindow::~MainWindow()
{
    if (m_machine != 0)
        m_machine->stop();

    writeSettings();

    delete ui;
    delete m_spectrum;
}

void MainWindow::readSettings()
{
    QString inputDeviceName = m_settings.value("soundcard/input","").toString();
    QString outputDeviceName = m_settings.value("soundcard/output","").toString();
    float samplerate = m_settings.value("soundcard/samplerate", 44100.0f).toFloat();

    PaDeviceIndex inDevice = PA_Helper::getDeviceIndexByName(inputDeviceName);
    PaDeviceIndex outDevice = PA_Helper::getDeviceIndexByName(outputDeviceName);
    m_machine->setupSoundcard(inDevice, outDevice, samplerate);
    m_spectrum->setSampleRate(samplerate);
    m_scope->setSampleRate(samplerate);

    qDebug() << "Loading settings.. ";
    qDebug() << "input device : " << inputDeviceName;
    qDebug() << "output device: " << outputDeviceName;
    qDebug() << "sample rate  : " << samplerate;

    QVariant sizeVariant = m_settings.value("mainwindow/size");
    if (!sizeVariant.isNull())
    {
        resize(sizeVariant.toSize());
    }

    m_lastDirectory = m_settings.value("lastdir", "").toString();
    m_lastAudioDirectory = m_settings.value("lastaudiodir","").toString();
}

void MainWindow::writeSettings()
{
    QString inputDevice = PA_Helper::getDeviceString(m_machine->getInputDevice());
    QString outputDevice = PA_Helper::getDeviceString(m_machine->getOutputDevice());
    m_settings.setValue("soundcard/input",inputDevice);
    m_settings.setValue("soundcard/output",outputDevice);
    m_settings.setValue("soundcard/samplerate", m_machine->getSamplerate());

    m_settings.setValue("mainwindow/size", size());

    m_settings.setValue("lastdir", m_lastDirectory);
    m_settings.setValue("lastaudiodir", m_lastAudioDirectory);
}

bool MainWindow::save()
{
    if (m_filepath=="")
    {
        m_filepath = askForFilename();
        if (m_filepath == "")
            return false; // user cancelled
    }

    // try to save
    if (saveScriptFile(m_filepath))
    {
        return true;
    }

    // save failed ..
    // prompt user for different file name
    do
    {
        QMessageBox msgBox;
        QString msg("Error writing file -- permissions?");
        msgBox.setText(msg);
        msgBox.exec();

    } while ((m_filepath != "") || (saveScriptFile(m_filepath)==false));
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // check if we need to save the BasicDSP script
    const QTextDocument *doc = m_sourceEditor->document();
    if (doc->isModified() || (m_filepath.isEmpty() && !doc->isEmpty()))
    {
        QMessageBox msgBox;
        QString msg("Closing BasicDSP - Do you want to save the changes?");
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        switch(msgBox.exec())
        {
        case QMessageBox::Save:
            if (!save())
            {
                // file not saved
                // user cancelled
                event->ignore();
            }
            else
            {
                event->accept();
            }
            writeSettings();
            break;
        case QMessageBox::Discard:
            writeSettings();
            event->accept();
            break;
        default:
        case QMessageBox::Cancel:
            event->ignore();
            break;
        }
    }
}

QString MainWindow::askForFilename()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"),
                               m_lastDirectory,
                               tr("BasicDSP files (*.dsp)"));
    return filePath;
}

bool MainWindow::saveScriptFile(const QString &filePath)
{
    QFile file;
    file.setFileName(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    QTextStream stream(&file);
    stream << m_sourceEditor->toPlainText();
    qDebug() << m_sourceEditor->toPlainText();
    stream.flush();
    file.flush();
    file.close();
    m_filepath = filePath;

    QTextDocument *doc = m_sourceEditor->document();
    QFileInfo info(filePath);
    doc->setModified(false);
    m_lastDirectory = info.path();
    updateBasicDSPWindowTitle();
    return true;
}

void MainWindow::updateBasicDSPWindowTitle()
{
    QString title("BasicDSP 2.0 ");
    QFileInfo info(m_filepath);
    QString fn = info.fileName();
    if (!fn.isEmpty())
    {
        title.append("[");
        title.append(fn);
        title.append("]");
    }
    setWindowTitle(title);
}

void MainWindow::on_GUITimer()
{
    float L,R;
    m_machine->getVU(L,R);
    m_leftVUMeter->setLevel(L);
    m_rightVUMeter->setLevel(R);
    m_leftVUMeter->update();
    m_rightVUMeter->update();

    // read data streams from virtual machine
    // and process/send to the scope and/or spectrum displays

    // **********************************************************************
    // Scope
    // **********************************************************************
    PaUtilRingBuffer* rbPtr = m_machine->getRingBufferPtr(0);

    ring_buffer_size_t items = PaUtil_GetRingBufferReadAvailable(rbPtr);
    while (items >= 256)
    {
        VirtualMachine::ring_buffer_data_t data[256];
        PaUtil_ReadRingBuffer(rbPtr, data, 256);
        if (!m_scope->isHidden())
            m_scope->submit256Samples(data);
        items = PaUtil_GetRingBufferReadAvailable(rbPtr);
    }
    m_scope->update();

    // **********************************************************************
    // Spectrum
    // **********************************************************************
    rbPtr = m_machine->getRingBufferPtr(1);
    items = PaUtil_GetRingBufferReadAvailable(rbPtr);
    while (items >= 256)
    {
        VirtualMachine::ring_buffer_data_t data[256];
        PaUtil_ReadRingBuffer(rbPtr, data, 256);
        if (!m_spectrum->isHidden())
        {
            m_spectrum->submit256Samples(data);
        }
        items = PaUtil_GetRingBufferReadAvailable(rbPtr);
    }
    m_spectrum->update();
}

void MainWindow::scopeChannelChanged(uint32_t channelID)
{
    qDebug() << "scopeChannelChanged() " << channelID;
    std::string varname = m_scope->getChannelName(channelID);
    m_machine->setMonitoringVariable(0, channelID, varname);
}

void MainWindow::spectrumChannelChanged(uint32_t channelID)
{
    qDebug() << "spectrumChannelChanged() " << channelID;
    std::string varname = m_spectrum->getChannelName(channelID);
    m_machine->setMonitoringVariable(1, channelID, varname);
}

bool MainWindow::compileAndRun()
{
    Parser    parser;
    Tokenizer tokenizer;

    if (m_machine->isRunning())
        m_machine->stop();

    QScopedPointer<Reader> reader(Reader::create(m_sourceEditor->toPlainText()));
    if (reader.isNull())
    {
        // error, probably due to an empty source code editor
        ui->statusBar->showMessage("Error: no source code?");
        return false;
    }

    std::vector<token_t> tokens;
    bool ok = tokenizer.process(reader.data(), tokens);
    if (!ok)
    {
        // tokenize error
        QString errstr("Tokenizer error: ");
        errstr.append(tokenizer.getErrorString().c_str());
        ui->statusBar->showMessage(errstr);

        Reader::position_info pos = tokenizer.getErrorPosition();
        m_sourceEditor->setErrorLine(pos.line+1);
        return false;
    }
    else
    {
        qDebug() << "Tokenizer produced " << tokens.size() << " tokens";
    }

    qDebug() << "-- TOKENS -- ";
    for(uint32_t i=0; i<tokens.size(); i++)
    {
        qDebug() << tokens[i].tokID;
    }

    ParseContext context;
    bool parseOK = parser.process(tokens, context);

    // write AST to the debug console
    qDebug() << "-- PARSE TREE -- ";
    std::stringstream ss;
    auto statementIterator = context.getStatements().begin();
    while(statementIterator != context.getStatements().end())
    {
        (*statementIterator)->dump(ss,context.m_variables,0);
        statementIterator++;
    }
    qDebug() << ss.str().c_str();

    if (!parseOK)
    {
        Reader::position_info pos = parser.getLastErrorPos();
        QString txt = QString("Program error on line %1: ").arg(pos.line+1);
        txt.append(parser.getLastError().c_str());
        ui->statusBar->showMessage(txt);
        m_sourceEditor->setErrorLine(pos.line+1);
        return false;
    }
    else
    {
        ui->statusBar->showMessage("Program accepted!");
        m_sourceEditor->setErrorLine(0);
    }

    VM::program_t program;
    VM::variables_t vars;
    if (!ASTToVM::process(context, program, vars))
    {
        ui->statusBar->showMessage("AST conversion failed!");
        qDebug() << "AST conversion failed! :(";
        return false;
    }
    else
    {
        ss.clear();
        if (m_machine != 0)
        {
            // dump the program for debugging and run!
            std::stringstream ss;
            m_machine->stop();
            m_machine->loadProgram(program, vars);
            m_machine->dump(ss);
            m_machine->setSlider(0, m_slider1->getValue());
            m_machine->setSlider(1, m_slider2->getValue());
            m_machine->setSlider(2, m_slider3->getValue());
            m_machine->setSlider(3, m_slider4->getValue());
            m_machine->setFrequency(ui->freqSlider->value());

            m_machine->setMonitoringVariable(0,0,m_scope->getChannelName(0));
            m_machine->setMonitoringVariable(0,1,m_scope->getChannelName(1));
            m_machine->setMonitoringVariable(1,0,m_spectrum->getChannelName(0));
            m_machine->setMonitoringVariable(1,1,m_spectrum->getChannelName(1));

            m_machine->start();
            qDebug() << ss.str().c_str();
            qDebug() << " - Variables -";
            for(size_t i=0; i<vars.size(); i++)
            {
                qDebug() << vars[i].m_name.c_str();
            }
            return true;
        }
    }
    return false;
}

QString MainWindow::openAudioFile()
{
    return  QFileDialog::getOpenFileName(this,
        tr("Open audio file"), m_lastAudioDirectory, tr("Audio Files (*.wav)"));
}

void MainWindow::on_actionExit_triggered()
{
    //TODO: check for unsaved things
    //      quit any running threads.

    close();
}

void MainWindow::on_runButton_clicked()
{
    // sanity
    if (m_machine == 0)
        return;

    if (!m_machine->isRunning())
    {
        // user pressed RUN
        bool ok = compileAndRun();
        if (ok)
        {
            ui->runButton->setText("Stop");
            ui->recompileButton->setEnabled(true);
        }
        else
        {
            // compile failed, don't run
            ui->runButton->setText("Run");
            ui->recompileButton->setEnabled(false);
        }
    }
    else
    {
        // user pressed STOP
        ui->runButton->setText("Run");
        ui->recompileButton->setEnabled(false);
        m_machine->stop();
    }
}

void MainWindow::on_Slider1Changed(float value)
{
    if (m_machine != 0)
    {
        m_machine->setSlider(0,value);
    }
}

void MainWindow::on_Slider2Changed(float value)
{
    if (m_machine != 0)
    {
        m_machine->setSlider(1,value);
    }
}

void MainWindow::on_Slider3Changed(float value)
{
    if (m_machine != 0)
    {
        m_machine->setSlider(2,value);
    }
}

void MainWindow::on_Slider4Changed(float value)
{
    if (m_machine != 0)
    {
        m_machine->setSlider(3,value);
    }
}

void MainWindow::on_SourceChanged()
{
    if (m_machine == 0)
        return;

    if (ui->inputAudioFile->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_WAV);
        ui->freqLineEdit->setEnabled(false);
        ui->freqSlider->setEnabled(false);
    }
    if (ui->inputSineWave->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_SINE);
        ui->freqLineEdit->setValidator(new QDoubleValidator(0,m_machine->getSamplerate(),3));
        ui->freqSlider->setRange(0,m_machine->getSamplerate());
        ui->freqLineEdit->setEnabled(true);
        ui->freqSlider->setEnabled(true);
    }
    if (ui->inputQuadSine->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_QUADSINE);
        ui->freqLineEdit->setValidator(new QDoubleValidator(-m_machine->getSamplerate()/2,m_machine->getSamplerate()/2,3));
        ui->freqSlider->setRange(-m_machine->getSamplerate()/2,m_machine->getSamplerate()/2);
        ui->freqLineEdit->setEnabled(true);
        ui->freqSlider->setEnabled(true);
    }
    if (ui->inputWhiteNoise->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_NOISE);
        ui->freqLineEdit->setEnabled(false);
        ui->freqSlider->setEnabled(false);
    }
    if (ui->inputImpulse->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_IMPULSE);
        ui->freqLineEdit->setEnabled(false);
        ui->freqSlider->setEnabled(false);
    }
    if (ui->inputSoundcard->isChecked())
    {
        m_machine->setSource(VirtualMachine::SRC_SOUNDCARD);
        ui->freqLineEdit->setEnabled(false);
        ui->freqSlider->setEnabled(false);
    }
}

void MainWindow::on_actionSoundcard_triggered()
{
    // open sound setup dialog
    SoundcardDialog *dialog = new SoundcardDialog(this);

    // fill dialog with info
    dialog->setSamplerate(m_machine->getSamplerate());
    dialog->setInputSource(m_machine->getInputDevice());
    dialog->setOutputSource(m_machine->getOutputDevice());

    if (dialog->exec() == 1)
    {
        m_machine->setupSoundcard(dialog->getInputSource(),
                                  dialog->getOutputSource(),
                                  dialog->getSamplerate());

        m_spectrum->setSampleRate(dialog->getSamplerate());
        m_scope->setSampleRate(dialog->getSamplerate());
    }
    delete dialog;
}



void MainWindow::on_scopeButton_clicked()
{
    if (m_scope->isHidden())
        m_scope->show();
    else
        m_scope->hide();
}

void MainWindow::on_actionFont_triggered()
{
    QFontDialog *dialog = new QFontDialog(this);
    dialog->setCurrentFont(m_sourceEditor->font());
    int result = dialog->exec();
    if (result == 1)
    {
        m_sourceEditor->setFont(dialog->selectedFont());
    }
    delete dialog;
}

void MainWindow::on_pushButton_clicked()
{
    if (m_spectrum->isHidden())
        m_spectrum->show();
    else
        m_spectrum->hide();
}

void MainWindow::on_actionSave_triggered()
{
    // save the file, prompting the user
    // for a filename if m_filepath is
    // empty
    save();
}


void MainWindow::on_actionSave_As_triggered()
{
    // call save with m_filepath set to ""
    // thus triggering an immediate prompt
    // for a file name

    QString oldFilepath = m_filepath;
    m_filepath = "";
    if (!save())
    {
        // user cancelleds
        m_filepath = oldFilepath;
    }
}

void MainWindow::on_action_Open_triggered()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    m_lastDirectory,
                                                    tr("BasicDSP files (*.dsp)"));
    if (filepath == "")
        return; // user cancelled

    QFile file;
    file.setFileName(filepath);
    QFileInfo info(filepath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        QString txt("Cannot open file ");
        txt.append(info.fileName());
        msgBox.setText(txt);
        msgBox.exec();
        return;
    }

    m_sourceEditor->clear();
    m_sourceEditor->setPlainText(file.readAll());
    m_sourceEditor->document()->setModified(false);
    file.close();
    m_filepath = filepath;
    m_lastDirectory = info.path();
    updateBasicDSPWindowTitle();
}

void MainWindow::on_actionClear_triggered()
{
    m_sourceEditor->clear();
}

void MainWindow::on_recompileButton_clicked()
{
    // user pressed RUN
    bool ok = compileAndRun();
    if (ok)
    {
        ui->runButton->setText("Stop");
        ui->recompileButton->setEnabled(true);
    }
    else
    {
        // compile failed, don't run
        ui->runButton->setText("Run");
        ui->recompileButton->setEnabled(false);
    }
}

void MainWindow::on_freqLineEdit_editingFinished()
{
    bool ok;
    QString valstr = ui->freqLineEdit->text();
    double value = valstr.toDouble(&ok);
    if (ok)
    {
        ui->freqSlider->setValue((int)value);
        m_machine->setFrequency(value);
    }
}

void MainWindow::on_freqSlider_valueChanged(int value)
{
    QString num= QString::number(value);
    ui->freqLineEdit->setText(num);
    m_machine->setFrequency(value);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog *dialog = new AboutDialog(this);
    dialog->exec();
}

void MainWindow::on_actionAudio_file_triggered()
{
    if (m_machine != 0)
    {
        QString filename = openAudioFile();
        if (m_machine->setAudioFile(filename))
        {
            QFileInfo info(filename);
            m_lastAudioDirectory = info.path();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("There was a problem loading the WAV file\nWrong format? I need a 2-channel/stereo file!");
            msgBox.exec();
        }
    }
}
