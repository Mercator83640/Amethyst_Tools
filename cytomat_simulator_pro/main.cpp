#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("CytomatSimulator");
    QApplication::setOrganizationName("ADRETEK");

    MainWindow w;
    w.show();

    return a.exec();
}
