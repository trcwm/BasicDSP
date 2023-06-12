#include <QDebug>
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{    
    ui->setupUi(this);
    ui->imageLabel->setPixmap(QPixmap(":/images/logo.png"));

    QString versionString = QString::asprintf("%s compiled on %s", BASICDSPVERSIONSTRING, __DATE__);
    ui->versionInfo->setText(versionString);

    setWindowTitle("About BasicDSP");
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

