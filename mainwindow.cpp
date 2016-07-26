#include <QCoreApplication>
#include <QDebug>
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
