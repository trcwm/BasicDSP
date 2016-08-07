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
