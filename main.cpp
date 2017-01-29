#include "mainwindow.h"
#include <QApplication>
#include <QPixmap>
#include <QThread>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("MoseleyInstruments");
    QCoreApplication::setOrganizationDomain("www.moseleyinstruments.com");
    QCoreApplication::setApplicationName("BasicDSP");

    QPixmap pixmap(":/images/logo.png");
    QSplashScreen splash(pixmap);
    splash.show();
    app.processEvents();
    QThread::msleep(1000);

    MainWindow w;
    w.show();    
    splash.finish(&w);

    return app.exec();
}
