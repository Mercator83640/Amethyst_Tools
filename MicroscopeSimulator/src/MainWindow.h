#pragma once

#include <QMainWindow>
#include <QUdpSocket>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum class Status_Stage : int
{
    FREE = 0,
    OCCUPIED = 1,
    RELEASE = 2
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartStop();
    void onPlateReadyRead();
    void onAckReadyRead();
    void onSendPeriodic();
    void onSetFree();
    void onSetOccupied();
    void onSetRelease();

private:
    void logLine(const QString & s);
    void setRunningUi(bool running);
    void setStatus(Status_Stage st, bool sendNow = true);
    void sendStatusDatagram();
    void handlePlateDatagram(const QByteArray &dat, const QHostAddress &sender, quint16 senderPort);
    bool bindPlateSocket();
    void clearPlateFields();

    Ui::MainWindow *ui;

    QUdpSocket * m_plateRx = nullptr;
    QUdpSocket * m_ackRx = nullptr;     // listens for ACK from Schedule
    QUdpSocket * m_statusTx = nullptr;
    QTimer m_periodicTimer;
    QTimer m_releaseFallbackTimer;

    Status_Stage m_status = Status_Stage::FREE;

    bool m_running = false;
};
