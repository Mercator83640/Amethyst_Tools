#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QHostAddress>

static QString ts() { return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    m_plateRx = new QUdpSocket(this);
    m_ackRx = new QUdpSocket(this);
    m_statusTx = new QUdpSocket(this);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartStop);
    connect(ui->btnFree, &QPushButton::clicked, this, &MainWindow::onSetFree);
    connect(ui->btnOccupied, &QPushButton::clicked, this, &MainWindow::onSetOccupied);
    connect(ui->btnRelease, &QPushButton::clicked, this, &MainWindow::onSetRelease);

    connect(&m_periodicTimer, &QTimer::timeout, this, &MainWindow::onSendPeriodic);
    connect(m_plateRx, &QUdpSocket::readyRead, this, &MainWindow::onPlateReadyRead);
    connect(m_ackRx, &QUdpSocket::readyRead, this, &MainWindow::onAckReadyRead);

    setStatus(Status_Stage::FREE, false);
    setRunningUi(false);

    logLine(QString("[%1] Simulator ready. Default status = FREE (0).").arg(ts()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logLine(const QString &s)
{
    ui->log->appendPlainText(s);
}

void MainWindow::setRunningUi(bool running)
{
    m_running = running;
    ui->btnStart->setText(running ? "Arrêter" : "Démarrer");
    ui->labelRun->setText(QString("État : %1").arg(running ? "en cours" : "arrêté"));
}

bool MainWindow::bindPlateSocket()
{
    const int platePort = ui->spinPlatePort->value();
    const bool any = (ui->comboBind->currentIndex() == 0);

    m_plateRx->close();

    const QHostAddress bindAddr = any ? QHostAddress::AnyIPv4 : QHostAddress::LocalHost;

    const bool ok = m_plateRx->bind(bindAddr,
                                    quint16(platePort),
                                    QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if (!ok) {
        logLine(QString("[%1] ERROR bind plate socket on %2:%3 -> %4")
                .arg(ts(), bindAddr.toString(), QString::number(platePort), m_plateRx->errorString()));
        return false;
    }

    logLine(QString("[%1] Listening for plates on %2:%3 (UDP)")
            .arg(ts(), bindAddr.toString(), QString::number(platePort)));
    return true;
}

void MainWindow::onStartStop()
{
    if (!m_running) {
        if (!bindPlateSocket()) {
            setRunningUi(false);
            return;
        }

        // Bind ACK socket (Schedule will send an acknowledgement here)
        const int ackPort = ui->spinAckPort->value();
        m_ackRx->close();
        const bool any = (ui->comboBind->currentIndex() == 0);
        const QHostAddress bindAddr = any ? QHostAddress::AnyIPv4 : QHostAddress::LocalHost;
        const bool okAck = m_ackRx->bind(bindAddr,
                                         quint16(ackPort),
                                         QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        if (!okAck) {
            logLine(QString("[%1] ERROR bind ACK socket on %2:%3 -> %4")
                    .arg(ts(), bindAddr.toString(), QString::number(ackPort), m_ackRx->errorString()));
        } else {
            logLine(QString("[%1] Listening for ACK on %2:%3 (UDP)")
                    .arg(ts(), bindAddr.toString(), QString::number(ackPort)));
        }

        if (ui->checkPeriodic->isChecked()) {
            m_periodicTimer.start(ui->spinPeriodMs->value());
            logLine(QString("[%1] Periodic status enabled every %2 ms.")
                    .arg(ts(), QString::number(ui->spinPeriodMs->value())));
        } else {
            m_periodicTimer.stop();
            logLine(QString("[%1] Periodic status disabled.").arg(ts()));
        }

        setRunningUi(true);
        sendStatusDatagram(); // initial status
    } else {
        m_plateRx->close();
        m_ackRx->close();
        m_periodicTimer.stop();
        m_releaseFallbackTimer.stop();
        setRunningUi(false);
        logLine(QString("[%1] Stopped.").arg(ts()));
    }
}

void MainWindow::onPlateReadyRead()
{
    while (m_plateRx->hasPendingDatagrams()) {
        QByteArray dat;
        dat.resize(int(m_plateRx->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;
        m_plateRx->readDatagram(dat.data(), dat.size(), &sender, &senderPort);
        handlePlateDatagram(dat, sender, senderPort);
    }
}

void MainWindow::handlePlateDatagram(const QByteArray &dat, const QHostAddress &sender, quint16 senderPort)
{
    QJsonParseError perr;
    const auto doc = QJsonDocument::fromJson(dat, &perr);
    if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        logLine(QString("[%1] Plate datagram ignored (invalid JSON) from %2:%3 : %4")
                .arg(ts(), sender.toString(), QString::number(senderPort), QString::fromUtf8(dat)));
        return;
    }

    const QJsonObject o = doc.object();

    // ✅ Nouveau protocole : Cmd
    const QString cmd = o.value("Cmd").toString().trimmed();

    // ----- ABORT / STOP -----
    if (cmd.compare("ABORT", Qt::CaseInsensitive) == 0) {
        const QString cb = o.value("CB").toString();

        logLine(QString("[%1] ABORT received from %2:%3 -> CB=%4 (currentStatus=%5)")
                    .arg(ts(), sender.toString(), QString::number(senderPort), cb, QString::number(int(m_status))));

        // règle demandée: si OCCUPIED -> RELEASE
        if (m_status == Status_Stage::OCCUPIED) {
            setStatus(Status_Stage::RELEASE, true);
            logLine(QString("[%1] ABORT triggers transition: OCCUPIED -> RELEASE (waiting ACK_RELEASE)")
                        .arg(ts()));
        } else {
            logLine(QString("[%1] ABORT ignored (status not OCCUPIED)").arg(ts()));
        }
        return;
    }

    // ----- START_ANALYSIS (nouveau) ou compatibilité ancien JSON -----
    // Ancien payload: {CB, Date, DualAnalysis}
    // Nouveau payload: {Cmd:"START_ANALYSIS", CB, Date, DualAnalysis, TsUtc}
    if (!cmd.isEmpty() && cmd.compare("START_ANALYSIS", Qt::CaseInsensitive) != 0) {
        logLine(QString("[%1] Cmd ignored (unsupported) from %2:%3 : %4")
                    .arg(ts(), sender.toString(), QString::number(senderPort), cmd));
        return;
    }

    const QString cb = o.value("CB").toString();
    const QString dateIso = o.value("Date").toString();
    const bool dual = o.value("DualAnalysis").toBool(false);

    ui->editCB->setText(cb);
    ui->editDate->setText(dateIso);
    ui->checkDual->setChecked(dual);

    logLine(QString("[%1] Plate received from %2:%3 -> CB=%4 Date=%5 DualAnalysis=%6")
            .arg(ts(), sender.toString(), QString::number(senderPort), cb, dateIso, dual ? "true" : "false"));

    // Rule: on plate reception -> OCCUPIED
    setStatus(Status_Stage::OCCUPIED, true);
}

void MainWindow::setStatus(Status_Stage st, bool sendNow)
{
    m_status = st;

    QString label;
    switch (st) {
        case Status_Stage::FREE:     label = "FREE (0)"; break;
        case Status_Stage::OCCUPIED: label = "OCCUPIED (1)"; break;
        case Status_Stage::RELEASE:  label = "RELEASE (2)"; break;
    }
    ui->labelStatusValue->setText(label);

    if (sendNow && m_running) {
        sendStatusDatagram();
    }
}

void MainWindow::sendStatusDatagram()
{
    const QString host = ui->editStatusHost->text().trimmed();
    const quint16 port = quint16(ui->spinStatusPort->value());

    QHostAddress target;
    if (!target.setAddress(host)) {
        if (host.compare("localhost", Qt::CaseInsensitive) == 0) {
            target = QHostAddress::LocalHost;
        } else {
            logLine(QString("[%1] ERROR invalid status host: %2").arg(ts(), host));
            return;
        }
    }

    const QByteArray datagram = QByteArray::number(int(m_status)); // "0"/"1"/"2"
    const qint64 sent = m_statusTx->writeDatagram(datagram, target, port);

    if (sent < 0) {
        logLine(QString("[%1] ERROR send status to %2:%3 -> %4")
                .arg(ts(), target.toString(), QString::number(port), m_statusTx->errorString()));
    } else {
        logLine(QString("[%1] Status sent to %2:%3 : %4")
                .arg(ts(), target.toString(), QString::number(port), QString::fromUtf8(datagram)));
    }
}

void MainWindow::onSendPeriodic()
{
    if (!m_running) return;
    if (!ui->checkPeriodic->isChecked()) return;
    sendStatusDatagram();
}

void MainWindow::onSetFree()
{
    setStatus(Status_Stage::FREE, true);
    clearPlateFields();
}

void MainWindow::onSetOccupied()
{
    setStatus(Status_Stage::OCCUPIED, true);
}

void MainWindow::onSetRelease()
{
    setStatus(Status_Stage::RELEASE, true);
    logLine(QString("[%1] RELEASE pressed. Waiting for ACK from Schedule...").arg(ts()));

    // Fallback: if no ACK arrives, automatically go back to FREE after 10 seconds
    // m_releaseFallbackTimer.stop();
    // connect(&m_releaseFallbackTimer, &QTimer::timeout, this, [this](){
    //     if (m_status == Status_Stage::RELEASE) {
    //         logLine(QString("[%1] No ACK after 10s: RELEASE -> FREE (fallback)").arg(ts()));
    //         setStatus(Status_Stage::FREE, true);
    //     }
    // });
    // m_releaseFallbackTimer.start(10000);
}


void MainWindow::onAckReadyRead()
{
    while (m_ackRx->hasPendingDatagrams()) {
        QByteArray dat;
        dat.resize(int(m_ackRx->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;
        m_ackRx->readDatagram(dat.data(), dat.size(), &sender, &senderPort);

        dat = dat.trimmed();
        const QString s = QString::fromUtf8(dat);

        // On ne déclenche le retour FREE que sur ACK_RELEASE (ou JSON ack release)
        bool isAckRelease = false;

        if (s == "ACK_RELEASE") {
            isAckRelease = true;
        } else {
            QJsonParseError perr;
            const auto doc = QJsonDocument::fromJson(dat, &perr);
            if (perr.error == QJsonParseError::NoError && doc.isObject()) {
                const auto o = doc.object();

                // ✅ Nouveau format:
                // {"Cmd":"ACK_RELEASE", ...}
                if (o.value("Cmd").toString() == "ACK_RELEASE") {
                    isAckRelease = true;
                }

                // (optionnel) si tu veux garder ton ancien format type/for :
                if (o.value("type").toString() == "ack" &&
                    o.value("for").toString() == "release") {
                    isAckRelease = true;
                }
            }
        }

        if (!isAckRelease) {
            logLine(QString("[%1] ACK datagram ignored (not ACK_RELEASE) from %2:%3 : %4")
                        .arg(ts(), sender.toString(), QString::number(senderPort), s));
            continue;
        }

        logLine(QString("[%1] ACK_RELEASE received from %2:%3 : %4")
                    .arg(ts(), sender.toString(), QString::number(senderPort), s));

        // ✅ Transition automatique uniquement si on est bien en RELEASE
        if (m_status == Status_Stage::RELEASE) {
            m_releaseFallbackTimer.stop();
            logLine(QString("[%1] ACK_RELEASE triggers transition: RELEASE -> FREE").arg(ts()));
            setStatus(Status_Stage::FREE, true);
            // ✅ nettoyer l'UI (plaque rangée)
            clearPlateFields();
            logLine(QString("[%1] UI cleared (CB/Date/DualAnalysis)").arg(ts()));
        } else {
            logLine(QString("[%1] ACK_RELEASE received but status is not RELEASE -> no transition").arg(ts()));
        }
    }
}

void MainWindow::clearPlateFields()
{
    ui->editCB->clear();
    ui->editDate->clear();
    ui->checkDual->setChecked(false);
}
