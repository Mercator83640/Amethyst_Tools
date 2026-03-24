#include "ScheduleSimulator.h"
#include "ui_ScheduleSimulator.h"

#include <QDate>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>

static QString ts()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

ScheduleSimulator::ScheduleSimulator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScheduleSimulator)
{
    ui->setupUi(this);

    m_statusRx = new QUdpSocket(this);
    m_plateTx  = new QUdpSocket(this);
    m_ackTx    = new QUdpSocket(this);

    connect(ui->btnStart, &QPushButton::clicked, this, &ScheduleSimulator::onStartStop);
    connect(ui->btnSendPlate, &QPushButton::clicked, this, &ScheduleSimulator::onSendPlate);
    connect(ui->btnSendAck, &QPushButton::clicked, this, &ScheduleSimulator::onSendAck);
    connect(ui->btnAbort, &QPushButton::clicked, this, &ScheduleSimulator::onSendAbort);
    connect(m_statusRx, &QUdpSocket::readyRead, this, &ScheduleSimulator::onStatusReadyRead);

    ui->editAcqHost->setText("127.0.0.1");
    ui->spinPlatePort->setValue(10332);
    ui->spinAckPort->setValue(10333);
    ui->spinStatusPort->setValue(12345);
    ui->checkAutoAck->setChecked(true);
    ui->labelStatusValue->setText("UNKNOWN");
    ui->labelLastCbValue->setText("-");
    ui->editCB->setText("INDJBNMP360BORDER1A");



    setRunningUi(false);
    logLine(QString("[%1] Schedule Simulator ready.").arg(ts()));
}

ScheduleSimulator::~ScheduleSimulator()
{
    delete ui;
}

void ScheduleSimulator::logLine(const QString &s)
{
    ui->log->appendPlainText(s);
}

void ScheduleSimulator::setRunningUi(bool running)
{
    m_running = running;
    ui->btnStart->setText(running ? "Arreter" : "Demarrer");
    ui->labelRun->setText(running ? "Etat : en ecoute" : "Etat : arrete");
    ui->btnSendPlate->setEnabled(running);
    ui->btnSendAck->setEnabled(running);
    ui->btnAbort->setEnabled(running);
}

bool ScheduleSimulator::bindStatusSocket()
{
    m_statusRx->close();

    const quint16 statusPort = quint16(ui->spinStatusPort->value());
    const bool ok = m_statusRx->bind(QHostAddress::AnyIPv4,
                                     statusPort,
                                     QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!ok) {
        logLine(QString("[%1] ERROR bind status socket on port %2 -> %3")
                .arg(ts(), QString::number(statusPort), m_statusRx->errorString()));
        return false;
    }

    logLine(QString("[%1] Listening status on UDP port %2")
            .arg(ts(), QString::number(statusPort)));
    return true;
}

void ScheduleSimulator::onStartStop()
{
    if (!m_running) {
        if (!bindStatusSocket()) {
            setRunningUi(false);
            return;
        }

        setRunningUi(true);
        logLine(QString("[%1] Simulator started.").arg(ts()));
    } else {
        m_statusRx->close();
        setRunningUi(false);
        logLine(QString("[%1] Simulator stopped.").arg(ts()));
    }
}

void ScheduleSimulator::onSendPlate()
{
    const QString cb = ui->editCB->text().trimmed();
    if (cb.isEmpty()) {
        logLine(QString("[%1] ERROR: CB vide.").arg(ts()));
        return;
    }

    sendPlateDatagram(cb);
}

void ScheduleSimulator::sendPlateDatagram(const QString &cb)
{
    const QString host = ui->editAcqHost->text().trimmed();
    const quint16 port = quint16(ui->spinPlatePort->value());

    QHostAddress target;
    if (!target.setAddress(host)) {
        if (host.compare("localhost", Qt::CaseInsensitive) == 0) {
            target = QHostAddress::LocalHost;
        } else {
            logLine(QString("[%1] ERROR invalid Acquisition host: %2").arg(ts(), host));
            return;
        }
    }

    QJsonObject o;
    o["Cmd"] = "START_ANALYSIS";
    o["CB"] = cb;
    o["Date"] = QDate::currentDate().toString(Qt::ISODate);
    o["DualAnalysis"] = false;
    o["TsUtc"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QByteArray dat = QJsonDocument(o).toJson(QJsonDocument::Compact);
    const qint64 sent = m_plateTx->writeDatagram(dat, target, port);

    if (sent < 0) {
        logLine(QString("[%1] ERROR send START_ANALYSIS to %2:%3 -> %4")
                .arg(ts(), target.toString(), QString::number(port), m_plateTx->errorString()));
        return;
    }

    ui->labelLastCbValue->setText(cb);
    logLine(QString("[%1] START_ANALYSIS sent to %2:%3 : %4")
            .arg(ts(), target.toString(), QString::number(port), QString::fromUtf8(dat)));
}

void ScheduleSimulator::onSendAck()
{
    sendAckRelease();
}

void ScheduleSimulator::sendAckRelease()
{
    const QString host = ui->editAcqHost->text().trimmed();
    const quint16 port = quint16(ui->spinAckPort->value());

    QHostAddress target;
    if (!target.setAddress(host)) {
        if (host.compare("localhost", Qt::CaseInsensitive) == 0) {
            target = QHostAddress::LocalHost;
        } else {
            logLine(QString("[%1] ERROR invalid Acquisition host: %2").arg(ts(), host));
            return;
        }
    }

    QJsonObject o;
    o["Cmd"] = "ACK_RELEASE";
    o["TsUtc"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QByteArray dat = QJsonDocument(o).toJson(QJsonDocument::Compact);
    const qint64 sent = m_ackTx->writeDatagram(dat, target, port);

    if (sent < 0) {
        logLine(QString("[%1] ERROR send ACK_RELEASE to %2:%3 -> %4")
                .arg(ts(), target.toString(), QString::number(port), m_ackTx->errorString()));
        return;
    }

    logLine(QString("[%1] ACK_RELEASE sent to %2:%3 : %4")
            .arg(ts(), target.toString(), QString::number(port), QString::fromUtf8(dat)));
}

void ScheduleSimulator::onSendAbort()
{
    sendAbort();
}

void ScheduleSimulator::sendAbort()
{
    const QString cb = ui->editCB->text().trimmed();
    if (cb.isEmpty()) {
        logLine(QString("[%1] ERROR: impossible d'envoyer ABORT, CB vide.").arg(ts()));
        return;
    }

    const QString host = ui->editAcqHost->text().trimmed();
    const quint16 port = quint16(ui->spinPlatePort->value());

    QHostAddress target;
    if (!target.setAddress(host)) {
        if (host.compare("localhost", Qt::CaseInsensitive) == 0) {
            target = QHostAddress::LocalHost;
        } else {
            logLine(QString("[%1] ERROR invalid Acquisition host: %2").arg(ts(), host));
            return;
        }
    }

    QJsonObject o;
    o["Cmd"] = "ABORT";
    o["CB"] = cb;
    o["TsUtc"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QByteArray dat = QJsonDocument(o).toJson(QJsonDocument::Compact);
    const qint64 sent = m_plateTx->writeDatagram(dat, target, port);

    if (sent < 0) {
        logLine(QString("[%1] ERROR send ABORT to %2:%3 -> %4")
                .arg(ts(), target.toString(), QString::number(port), m_plateTx->errorString()));
        return;
    }

    logLine(QString("[%1] ABORT sent to %2:%3 : %4")
            .arg(ts(), target.toString(), QString::number(port), QString::fromUtf8(dat)));
}

QString ScheduleSimulator::statusToString(const QString &s) const
{
    if (s == "0") return "FREE";
    if (s == "1") return "OCCUPIED";
    if (s == "2") return "RELEASE";
    return QString("UNKNOWN(%1)").arg(s);
}

void ScheduleSimulator::onStatusReadyRead()
{
    while (m_statusRx->hasPendingDatagrams()) {
        QByteArray dat;
        dat.resize(int(m_statusRx->pendingDatagramSize()));

        QHostAddress sender;
        quint16 senderPort = 0;
        m_statusRx->readDatagram(dat.data(), dat.size(), &sender, &senderPort);

        dat = dat.trimmed();
        const QString s = QString::fromUtf8(dat);

        logLine(QString("[%1] Status received from %2:%3 : %4 [%5]")
                .arg(ts(), sender.toString(), QString::number(senderPort), s, statusToString(s)));

        ui->labelStatusValue->setText(statusToString(s));
        m_lastStatus = s;

        if (s == "2" && ui->checkAutoAck->isChecked()) {
            logLine(QString("[%1] RELEASE detected -> auto ACK_RELEASE").arg(ts()));
            sendAckRelease();
        }
    }
}
