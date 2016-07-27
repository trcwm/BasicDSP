#include <QCoreApplication>
#include <QDebug>
#include <sstream>
#include "reader.h"
#include "tokenizer.h"
#include "parser.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /** set source editor font */
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->sourceEditor->setFont(fixedFont);

    /** add four sliders programatically */
    m_slider1 = new NamedSlider("slider 1", this);
    ui->mainLayout->addWidget(m_slider1);
    m_slider2 = new NamedSlider("slider 2", this);
    ui->mainLayout->addWidget(m_slider2);
    m_slider3 = new NamedSlider("slider 3", this);
    ui->mainLayout->addWidget(m_slider3);
    m_slider4 = new NamedSlider("slider 4", this);
    ui->mainLayout->addWidget(m_slider4);

    /** add VU meters */
    m_leftVUMeter = new VUMeter("Left", this);
    ui->mainLayout->addWidget(m_leftVUMeter);
    m_rightVUMeter = new VUMeter("Right", this);
    ui->mainLayout->addWidget(m_rightVUMeter);

    /** create the virtual machine */
    machine = new VirtualMachine(this);

    /** create a GUI timer to update VU etc */
    m_guiTimer = new QTimer(this);
    connect(m_guiTimer, SIGNAL(timeout()), this, SLOT(on_GUITimer()));
    m_guiTimer->start(100);

    machine->start();
}

MainWindow::~MainWindow()
{
    if (machine != 0)
        machine->stop();

    delete ui;
}

void MainWindow::on_GUITimer()
{
    float L,R;
    machine->getVU(L,R);
    m_leftVUMeter->setLevel(L);
    m_rightVUMeter->setLevel(R);
    m_leftVUMeter->update();
    m_rightVUMeter->update();
}

void MainWindow::on_actionExit_triggered()
{
    //TODO: check for unsaved things
    //      quit any running threads.
    QCoreApplication::exit(0);
}

void MainWindow::on_runButton_clicked()
{
    Parser    parser;
    Tokenizer tokenizer;

    Reader *reader = Reader::create(ui->sourceEditor->toPlainText());
    if (reader == 0)
    {
        // error, probably due to an empty source code editor
        qDebug() << "Error: no source code";
        return;
    }

    std::vector<token_t> tokens;
    bool ok = tokenizer.process(reader, tokens);
    if (!ok)
    {
        // tokenize error
        qDebug() << "Tokenizer error: " << tokenizer.getErrorString().c_str();
    }
    else
    {
        qDebug() << "Tokenizer produced " << tokens.size() << " tokens";
    }

    for(uint32_t i=0; i<tokens.size(); i++)
    {
        qDebug() << tokens[i].tokID;
    }

    statements_t statements;
    bool parseOK = parser.process(tokens, statements);

    std::stringstream ss;
    for(uint32_t i=0; i<statements.size(); i++)
    {
        statements[i]->dump(ss,0);
    }
    qDebug() << ss.str().c_str();

    if (!parseOK)
    {
        Reader::position_info pos = parser.getLastErrorPos();
        QString txt = QString("Program error on line %1: ").arg(pos.line+1);
        txt.append(parser.getLastError().c_str());
        ui->statusBar->showMessage(txt);
    }
    else
    {
        ui->statusBar->showMessage("Program accepted!");
    }
}
