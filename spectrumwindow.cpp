#include "spectrumwindow.h"
#include "ui_spectrumwindow.h"

SpectrumWindow::SpectrumWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SpectrumWindow)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
    ui->setupUi(this);

    m_spectrum = new SpectrumWidget(this);
    ui->mainLayout->addWidget(m_spectrum);
}

SpectrumWindow::~SpectrumWindow()
{
    delete ui;
}
