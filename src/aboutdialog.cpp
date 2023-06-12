
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    m_pixmap = new QPixmap(":/images/logo.png");
    if (m_pixmap != NULL)
        ui->imageLabel->setPixmap(*m_pixmap);

    QString versionString;
    versionString.asprintf("Version: %s compiled on %s", BASICDSPVERSIONSTRING, __DATE__);
    ui->versionInfo->setText(versionString);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

