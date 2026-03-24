#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("CytomatSimulator");
    QApplication::setOrganizationName("Adretek");

    MainWindow w;
    w.show();

    return a.exec();
}
