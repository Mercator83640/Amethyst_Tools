#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QElapsedTimer>
#include <QHostAddress>

static QString ts() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Sensible defaults
    ui->editHost->setText("10.0.0.12");
    ui->editDb->setText("ma_base");
    ui->editUser->setText("test_lan");

    connect(ui->btnTest, &QPushButton::clicked, this, &MainWindow::onTestClicked);

    // Show available drivers
    const auto drivers = QSqlDatabase::drivers();
    logLine(QString("[%1] Drivers Qt SQL disponibles: %2").arg(ts(), drivers.join(", ")));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::logLine(const QString& s) {
    ui->logOutput->appendPlainText(s);
}

void MainWindow::setStatusOk(const QString& s) {
    ui->labelStatus->setText("Statut : OK — " + s);
    ui->labelStatus->setStyleSheet("QLabel { color: #1b7f3a; font-weight: 600; }");
}

void MainWindow::setStatusKo(const QString& s) {
    ui->labelStatus->setText("Statut : KO — " + s);
    ui->labelStatus->setStyleSheet("QLabel { color: #b00020; font-weight: 600; }");
}

void MainWindow::onTestClicked() {
    const QString host = ui->editHost->text().trimmed();
    const int port = ui->spinPort->value();
    const QString dbname = ui->editDb->text().trimmed();
    const QString user = ui->editUser->text().trimmed();
    const QString pass = ui->editPass->text();
    const int timeoutSec = ui->spinTimeout->value();
    const bool doSelect1 = ui->checkSelect1->isChecked();

    ui->btnTest->setEnabled(false);

    logLine(QString("\n[%1] --- Test démarré ---").arg(ts()));
    logLine(QString("[%1] Cible: %2:%3  Base: %4  User: %5  Timeout: %6s")
            .arg(ts(), host, QString::number(port), dbname, user, QString::number(timeoutSec)));

    // Validate host lightly (optional)
    if (host.isEmpty()) {
        setStatusKo("Hôte/IP vide");
        logLine(QString("[%1] Erreur: hôte/IP manquant").arg(ts()));
        ui->btnTest->setEnabled(true);
        return;
    }

    // Each test uses its own connection name (important to avoid reuse issues)
    const QString connName = QString("healthcheck_%1").arg(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QElapsedTimer timer;
    timer.start();

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
        db.setHostName(host);
        db.setPort(port);
        db.setDatabaseName(dbname);
        db.setUserName(user);
        db.setPassword(pass);

        // MySQL connect options (works if the underlying plugin supports it)
        // - MYSQL_OPT_CONNECT_TIMEOUT is generally supported by QMYSQL.
        db.setConnectOptions(QString("MYSQL_OPT_CONNECT_TIMEOUT=%1").arg(timeoutSec));

        if (!db.open()) {
            const auto err = db.lastError();
            setStatusKo("connexion");
            logLine(QString("[%1] KO Connexion: %2").arg(ts(), err.text()));
            logLine(QString("[%1] Détails: nativeErrorCode=%2, driverText=%3")
                    .arg(ts(), err.nativeErrorCode(), err.driverText()));
        } else {
            logLine(QString("[%1] OK Connexion (%.0f ms)").arg(ts()).arg(timer.elapsed() * 1.0));
            if (doSelect1) {
                QSqlQuery q(db);
                if (!q.exec("SELECT 1")) {
                    const auto err = q.lastError();
                    setStatusKo("requête");
                    logLine(QString("[%1] KO Requête SELECT 1: %2").arg(ts(), err.text()));
                } else {
                    q.next();
                    const int v = q.value(0).toInt();
                    setStatusOk(QString("SELECT 1 = %1 (%.0f ms)").arg(v).arg(timer.elapsed() * 1.0));
                    logLine(QString("[%1] OK SELECT 1 -> %2 (%.0f ms)").arg(ts()).arg(v).arg(timer.elapsed() * 1.0));
                }
            } else {
                setStatusOk(QString("connexion uniquement (%.0f ms)").arg(timer.elapsed() * 1.0));
            }
            db.close();
        }
    }

    // IMPORTANT: remove the connection AFTER db goes out of scope
    QSqlDatabase::removeDatabase(connName);

    logLine(QString("[%1] --- Test terminé (%.0f ms) ---").arg(ts()).arg(timer.elapsed() * 1.0));
    ui->btnTest->setEnabled(true);
}
