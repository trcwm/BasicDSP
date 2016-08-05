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

void ScopeWindow::submit256Samples(float *buffer)
{
    m_scope->submit256Samples(buffer);
}
