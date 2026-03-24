#ifndef SCHEDULESIMULATOR_H
#define SCHEDULESIMULATOR_H

#include <QMainWindow>
#include <QUdpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class ScheduleSimulator;
}
QT_END_NAMESPACE

class ScheduleSimulator : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScheduleSimulator(QWidget *parent = nullptr);
    ~ScheduleSimulator();

private slots:
    void onStartStop();
    void onSendPlate();
    void onSendAck();
    void onSendAbort();
    void onStatusReadyRead();

private:
    void logLine(const QString &s);
    void setRunningUi(bool running);
    bool bindStatusSocket();
    void sendPlateDatagram(const QString &cb);
    void sendAckRelease();
    void sendAbort();
    QString statusToString(const QString &s) const;

private:
    Ui::ScheduleSimulator *ui;

    QUdpSocket *m_statusRx = nullptr;   // listen Acquisition status on 12345
    QUdpSocket *m_plateTx  = nullptr;   // send START_ANALYSIS / ABORT to 10332
    QUdpSocket *m_ackTx    = nullptr;   // send ACK_RELEASE to 10333

    bool m_running = false;
    QString m_lastStatus;
};

#endif // SCHEDULESIMULATOR_H
