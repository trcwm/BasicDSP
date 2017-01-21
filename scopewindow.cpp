#include "scopewindow.h"
#include "ui_scopewindow.h"

ScopeWindow::ScopeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScopeWindow)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    ui->setupUi(this);

    m_scope = new ScopeWidget(this);
    ui->mainLayout->addWidget(m_scope);

    m_hsizer = new QHBoxLayout(this);
    ui->mainLayout->addLayout(m_hsizer);

    m_chan1 = new QLineEdit(this);
    m_chan2 = new QLineEdit(this);
    m_hsizer->addWidget(m_chan1);
    m_hsizer->addWidget(m_chan2);

    connect(m_chan1, SIGNAL(textChanged(QString)), this, SLOT(chan1Changed()));
    connect(m_chan2, SIGNAL(textChanged(QString)), this, SLOT(chan2Changed()));
}

ScopeWindow::~ScopeWindow()
{
    delete ui;
}

void ScopeWindow::submit256Samples(VirtualMachine::ring_buffer_data_t *buffer)
{
    m_scope->submit256Samples(buffer);
}

void ScopeWindow::setSampleRate(float rate)
{
    m_scope->setSampleRate(rate);
}

std::string ScopeWindow::getChannelName(uint32_t channel)
{
    if (channel == 0)
        return m_chan1->text().toStdString();

    if (channel == 1)
        return m_chan2->text().toStdString();

    return std::string("");
}

void ScopeWindow::chan1Changed()
{
    emit channelChanged(0);
}

void ScopeWindow::chan2Changed()
{
    emit channelChanged(1);
}

