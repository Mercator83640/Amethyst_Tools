#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Microscope Simulator");
    a.setOrganizationName("LocalTools");
    a.setWindowIcon(QIcon(":/assets/app.png"));

    MainWindow w;
    w.show();
    return a.exec();
}
