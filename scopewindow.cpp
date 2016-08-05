#include "scopewindow.h"
#include "ui_scopewindow.h"

ScopeWindow::ScopeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScopeWindow)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    ui->setupUi(this);
}

ScopeWindow::~ScopeWindow()
{
    delete ui;
}
