#include "imagefilter_mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ImageFilter_MainWindow w;
//    w.show();
    w.showMaximized();
    return a.exec();
}
