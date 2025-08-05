
#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"
#include <utils.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    qInstallMessageHandler(myMessageHandler);
    Logger::instance().setEnabled(true);

#ifndef QT_DEBUG
    qDebug() << "Release mode";
    Logger::instance().setDebug(false);
#else
    qDebug() << "Debug mode";
    Logger::instance().setDebug(true);
#endif
    MainWindow window;
    window.show();
    return app.exec();
}
