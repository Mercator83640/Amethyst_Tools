#include "mainwindow.h"
#include "porttester.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

static QString nowStamp()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();

    m_tester = new PortTester(this);

    connect(m_btnLocal, &QPushButton::clicked, this, &MainWindow::onTestLocal);
    connect(m_btnRemote, &QPushButton::clicked, this, &MainWindow::onTestRemote);
    connect(m_btnCancel, &QPushButton::clicked, this, &MainWindow::onCancel);

    connect(m_tester, &PortTester::started, this, [this](PortTester::Mode mode){
        onStarted(static_cast<int>(mode));
    });

    connect(m_tester, &PortTester::finished, this, &MainWindow::onFinished);

    appendLog("Application prête.");
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi()
{
    setWindowTitle("Port Checker - Qt 5.15 (QTcpSocket/QTcpServer)");
    resize(820, 520);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // --- Inputs
    auto *box = new QGroupBox("Paramètres", central);
    auto *grid = new QGridLayout(box);

    grid->addWidget(new QLabel("Hôte (distance)"), 0, 0);
    m_hostEdit = new QLineEdit("127.0.0.1");
    grid->addWidget(m_hostEdit, 0, 1);

    grid->addWidget(new QLabel("Port"), 0, 2);
    m_portSpin = new QSpinBox();
    m_portSpin->setRange(1, 65535);
    m_portSpin->setValue(12345);
    grid->addWidget(m_portSpin, 0, 3);

    grid->addWidget(new QLabel("Famille (local)"), 1, 0);
    m_familyCombo = new QComboBox();
    m_familyCombo->addItem("Any (IPv4 + IPv6)", static_cast<int>(PortTester::AddressFamily::Any));
    m_familyCombo->addItem("IPv4 only", static_cast<int>(PortTester::AddressFamily::IPv4Only));
    m_familyCombo->addItem("IPv6 only", static_cast<int>(PortTester::AddressFamily::IPv6Only));
    grid->addWidget(m_familyCombo, 1, 1);

    grid->addWidget(new QLabel("Timeout (ms)"), 1, 2);
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setRange(100, 60000);
    m_timeoutSpin->setValue(1500);
    grid->addWidget(m_timeoutSpin, 1, 3);

    m_btnLocal = new QPushButton("Tester LOCAL (port libre)");
    m_btnRemote = new QPushButton("Tester DISTANCE (port ouvert)");
    m_btnCancel = new QPushButton("Annuler");
    m_btnCancel->setEnabled(false);

    auto *btnRow = new QHBoxLayout();
    btnRow->addWidget(m_btnLocal);
    btnRow->addWidget(m_btnRemote);
    btnRow->addWidget(m_btnCancel);

    grid->addLayout(btnRow, 2, 0, 1, 4);

    mainLayout->addWidget(box);

    // --- Results
    m_statusLabel = new QLabel("Statut : -");
    m_statusLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(m_statusLabel);

    auto *split = new QHBoxLayout();

    m_details = new QTextEdit();
    m_details->setReadOnly(true);
    m_details->setPlaceholderText("Détails du dernier test...");

    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->setPlaceholderText("Journal...");

    split->addWidget(m_details, 2);
    split->addWidget(m_log, 1);

    mainLayout->addLayout(split, 1);

    // Aide
    auto *help = new QLabel(
        "<b>Local</b> : tente un bind/listen avec QTcpServer. Si OK => port libre sur cette machine.<br/>"
        "<b>Distance</b> : tente une connexion avec QTcpSocket. Si connexion OK => port ouvert (service à l'écoute)." );
    help->setWordWrap(true);
    mainLayout->addWidget(help);
}

void MainWindow::appendLog(const QString &text)
{
    m_log->append(QString("[%1] %2").arg(nowStamp(), text));
}

void MainWindow::onTestLocal()
{
    const auto port = static_cast<quint16>(m_portSpin->value());
    const auto fam = static_cast<PortTester::AddressFamily>(m_familyCombo->currentData().toInt());

    appendLog(QString("Test local demandé : port=%1, famille=%2").arg(port).arg(m_familyCombo->currentText()));
    m_tester->testLocal(port, fam);
}

void MainWindow::onTestRemote()
{
    const auto host = m_hostEdit->text().trimmed();
    const auto port = static_cast<quint16>(m_portSpin->value());
    const auto timeout = m_timeoutSpin->value();

    appendLog(QString("Test distance demandé : %1:%2, timeout=%3ms").arg(host).arg(port).arg(timeout));
    m_tester->testRemote(host, port, timeout);
}

void MainWindow::onCancel()
{
    appendLog("Annulation demandée.");
    m_tester->cancel();
}

void MainWindow::onStarted(int mode)
{
    Q_UNUSED(mode);
    m_btnLocal->setEnabled(false);
    m_btnRemote->setEnabled(false);
    m_btnCancel->setEnabled(true);
    m_statusLabel->setText("Statut : test en cours...");
}

void MainWindow::onFinished(bool ok, const QString &summary, const QString &details)
{
    m_btnLocal->setEnabled(true);
    m_btnRemote->setEnabled(true);
    m_btnCancel->setEnabled(false);

    m_statusLabel->setText(QString("Statut : %1").arg(summary));
    m_statusLabel->setStyleSheet(ok ? "font-weight:bold;color:#177d2d;" : "font-weight:bold;color:#b00020;");

    m_details->setPlainText(details);
    appendLog(summary);
}
