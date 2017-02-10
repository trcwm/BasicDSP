
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#define __VERSION__ "2.0a"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    m_pixmap = new QPixmap(":/images/logo.png");
    if (m_pixmap != NULL)
        ui->imageLabel->setPixmap(*m_pixmap);

    ui->versionInfo->setText("Version: " __VERSION__ " compiled on " __DATE__);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

