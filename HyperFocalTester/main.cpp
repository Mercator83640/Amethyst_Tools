#include <QApplication>
#include "mainwindow.h"

#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Logger::install();   // <-- important

    MainWindow w;
    w.show();
    return app.exec();
}
