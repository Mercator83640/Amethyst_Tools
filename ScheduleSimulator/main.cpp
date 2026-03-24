#include "ScheduleSimulator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/icons/schedule.png"));
    QPixmap pm(":/icons/schedule.png");
qDebug() << "pixmap null =" << pm.isNull();

    ScheduleSimulator w;
    w.show();
    return a.exec();
}
