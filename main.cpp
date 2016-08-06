#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("MoseleyInstruments");
    QCoreApplication::setOrganizationDomain("www.moseleyinstruments.com");
    QCoreApplication::setApplicationName("BasicDSP");

    MainWindow w;
    w.show();

    return a.exec();
}
