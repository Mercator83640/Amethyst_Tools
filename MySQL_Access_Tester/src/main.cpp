#include <QApplication>
#include <QIcon>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName("MySQL Access Tester");
    a.setOrganizationName("LocalTools");

    // Fallback icon (also set by Windows resource)
    a.setWindowIcon(QIcon(":/assets/app.png"));

    MainWindow w;
    w.show();
    return a.exec();
}
