#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->editHost->setText("127.0.0.1");
    ui->spinPort->setRange(1, 65535);
    ui->spinPort->setValue(29127);
    ui->spinTimeout->setRange(50, 60000);
    ui->spinTimeout->setValue(800);

    ui->rbServer->setChecked(true);

    setVerdict("-", false, "Choisis un mode puis lance un test.");
    logLine("Ready.");
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::host() const { return ui->editHost->text().trimmed(); }
quint16 MainWindow::port() const { return static_cast<quint16>(ui->spinPort->value()); }
int MainWindow::timeoutMs() const { return ui->spinTimeout->value(); }

MainWindow::VerdictMode MainWindow::mode() const
{
    if (ui->rbClient->isChecked()) return VerdictMode::ClientRemoteOpen;
    if (ui->rbBoth->isChecked())   return VerdictMode::Both;
    return VerdictMode::ServerLocalFree;
}

void MainWindow::logLine(const QString &s)
{
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    ui->textLog->append(QString("[%1] %2").arg(ts, s));
}

void MainWindow::setVerdict(const QString& title, bool ok, const QString& details)
{
    ui->lblVerdictTitle->setText(title);
    ui->lblVerdict->setText(ok ? "OK ✅" : "ECHEC ❌");
    ui->lblDetails->setText(details);
}

bool MainWindow::isPortFreeLocal(quint16 p, QString& why)
{
    QTcpServer probe;
    if (!probe.listen(QHostAddress::Any, p)) {
        why = probe.errorString();
        return false;
    }
    probe.close();
    why = "Port libre (bind/listen OK).";
    return true;
}

bool MainWindow::canConnectRemote(const QString& h, quint16 p, int tMs, QString& why)
{
    QTcpSocket sock;
    QElapsedTimer timer;
    timer.start();

    sock.connectToHost(h, p);
    if (!sock.waitForConnected(tMs)) {
        why = QString("Connexion impossible (%1). elapsed=%2ms")
                  .arg(sock.errorString())
                  .arg(timer.elapsed());
        return false;
    }

    sock.disconnectFromHost();
    why = QString("Connexion OK. elapsed=%1ms").arg(timer.elapsed());
    return true;
}

void MainWindow::on_btnCheckLocal_clicked()
{
    const quint16 p = port();
    QString why;

    logLine(QString("LOCAL check: port=%1").arg(p));

    const bool ok = isPortFreeLocal(p, why);

    setVerdict("Local (port libre ?)", ok, why);
    logLine(ok ? ("OK: " + why) : ("FAIL: " + why));
}

void MainWindow::on_btnCheckRemote_clicked()
{
    const QString h = host();
    const quint16 p = port();
    const int tMs = timeoutMs();
    QString why;

    logLine(QString("REMOTE check: host=%1 port=%2 timeout=%3ms").arg(h).arg(p).arg(tMs));

    const bool ok = canConnectRemote(h, p, tMs, why);

    setVerdict("Remote (port ouvert/joignable ?)", ok, why);
    logLine(ok ? ("OK: " + why) : ("FAIL: " + why));
}

void MainWindow::on_btnFullCheck_clicked()
{
    const QString h = host();
    const quint16 p = port();
    const int tMs = timeoutMs();

    logLine(QString("FULL check: host=%1 port=%2 timeout=%3ms").arg(h).arg(p).arg(tMs));

    QString whyLocal, whyRemote;
    const bool localFree = isPortFreeLocal(p, whyLocal);
    const bool remoteOk  = canConnectRemote(h, p, tMs, whyRemote);

    QString details =
        QString("LocalFree=%1 (%2)\nRemoteOpen=%3 (%4)\nMode=%5")
            .arg(localFree ? "true" : "false")
            .arg(whyLocal)
            .arg(remoteOk ? "true" : "false")
            .arg(whyRemote);

    bool globalOk = false;
    QString title = "Verdict global";

    switch (mode()) {
    case VerdictMode::ServerLocalFree:
        globalOk = localFree;
        title = "Verdict global (Mode: Serveur = port local libre)";
        break;
    case VerdictMode::ClientRemoteOpen:
        globalOk = remoteOk;
        title = "Verdict global (Mode: Client = port distant ouvert)";
        break;
    case VerdictMode::Both:
        globalOk = localFree && remoteOk;
        title = "Verdict global (Mode: Both = local libre + distant ouvert)";
        break;
    }

    setVerdict(title, globalOk, details);
    logLine(globalOk ? "OK: Global verdict PASS." : "FAIL: Global verdict FAIL.");
}

void MainWindow::on_btnClear_clicked()
{
    ui->textLog->clear();
    setVerdict("-", false, "");
    logLine("Log cleared.");
}
