#include "mainwindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QColor>
#include <QDateTime>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QAbstractItemView>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_engine(this)
    , m_server(&m_engine, this)
{
    m_errorDescriptions.insert(0x01, "01 - Device busy");
    m_errorDescriptions.insert(0x02, "02 - Unknown command");
    m_errorDescriptions.insert(0x03, "03 - Telegram structure error");
    m_errorDescriptions.insert(0x04, "04 - Incorrect parameter");
    m_errorDescriptions.insert(0x05, "05 - Unknown location");
    m_errorDescriptions.insert(0x21, "21 - Handler occupied");
    m_errorDescriptions.insert(0x22, "22 - Handler empty");
    m_errorDescriptions.insert(0x31, "31 - Transfer station empty");
    m_errorDescriptions.insert(0x32, "32 - Transfer station occupied");

    buildUi();
    populateLocationsTable();
    refreshPorts();
    updateFromState();

    connect(&m_engine, &CytomatEngine::stateChanged,
            this, &MainWindow::updateFromState);

    connect(&m_server, &SerialServer::rxLine,
            this, &MainWindow::appendRx);
    connect(&m_server, &SerialServer::txLine,
            this, &MainWindow::appendTx);
    connect(&m_server, &SerialServer::portOpened,
            this, &MainWindow::onPortOpened);
    connect(&m_server, &SerialServer::portClosed,
            this, &MainWindow::onPortClosed);
    connect(&m_server, &SerialServer::portError,
            this, &MainWindow::onPortError);

    setWindowTitle("Cytomat Simulator");
    resize(1400, 900);
}

void MainWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    auto* mainLayout = new QVBoxLayout(central);

    auto* topLayout = new QHBoxLayout();
    m_portCombo = new QComboBox(this);
    m_refreshPortsButton = new QPushButton("Refresh", this);
    m_openPortButton = new QPushButton("Open", this);
    m_closePortButton = new QPushButton("Close", this);
    m_portStatusLabel = new QLabel("Port closed", this);

    topLayout->addWidget(new QLabel("Serial port:", this));
    topLayout->addWidget(m_portCombo, 1);
    topLayout->addWidget(m_refreshPortsButton);
    topLayout->addWidget(m_openPortButton);
    topLayout->addWidget(m_closePortButton);
    topLayout->addWidget(m_portStatusLabel);
    mainLayout->addLayout(topLayout);

    auto* middleLayout = new QHBoxLayout();

    auto* leftCol = new QVBoxLayout();
    auto* rightCol = new QVBoxLayout();
    middleLayout->addLayout(leftCol, 0);
    middleLayout->addLayout(rightCol, 1);

    auto* stateBox = new QGroupBox("State", this);
    auto* stateLayout = new QGridLayout(stateBox);

    auto makeReadOnlyCheck = [this]() {
        auto* cb = new QCheckBox(this);
        cb->setEnabled(false);
        return cb;
    };

    m_busyCheck = makeReadOnlyCheck();
    m_readyCheck = makeReadOnlyCheck();
    m_warningCheck = makeReadOnlyCheck();
    m_errorCheck = makeReadOnlyCheck();
    m_handlerCheck = makeReadOnlyCheck();
    m_transferCheck = makeReadOnlyCheck();
    m_gateCheck = makeReadOnlyCheck();
    m_doorCheck = makeReadOnlyCheck();

    m_warningCodeLabel = new QLabel("00", this);
    m_errorCodeLabel = new QLabel("00", this);
    m_actionCodeLabel = new QLabel("00", this);
    m_handlerPosLabel = new QLabel("Wait", this);
    m_swapLabel = new QLabel("100", this);

    stateLayout->addWidget(new QLabel("Busy", this), 0, 0);
    stateLayout->addWidget(m_busyCheck, 0, 1);
    stateLayout->addWidget(new QLabel("Ready", this), 1, 0);
    stateLayout->addWidget(m_readyCheck, 1, 1);
    stateLayout->addWidget(new QLabel("Warning", this), 2, 0);
    stateLayout->addWidget(m_warningCheck, 2, 1);
    stateLayout->addWidget(new QLabel("Error", this), 3, 0);
    stateLayout->addWidget(m_errorCheck, 3, 1);
    stateLayout->addWidget(new QLabel("Handler occupied", this), 4, 0);
    stateLayout->addWidget(m_handlerCheck, 4, 1);
    stateLayout->addWidget(new QLabel("Transfer occupied", this), 5, 0);
    stateLayout->addWidget(m_transferCheck, 5, 1);
    stateLayout->addWidget(new QLabel("Gate open", this), 6, 0);
    stateLayout->addWidget(m_gateCheck, 6, 1);
    stateLayout->addWidget(new QLabel("Device door open", this), 7, 0);
    stateLayout->addWidget(m_doorCheck, 7, 1);

    stateLayout->addWidget(new QLabel("Warning code", this), 0, 2);
    stateLayout->addWidget(m_warningCodeLabel, 0, 3);
    stateLayout->addWidget(new QLabel("Error code", this), 1, 2);
    stateLayout->addWidget(m_errorCodeLabel, 1, 3);
    stateLayout->addWidget(new QLabel("Action code", this), 2, 2);
    stateLayout->addWidget(m_actionCodeLabel, 2, 3);
    stateLayout->addWidget(new QLabel("Handler position", this), 3, 2);
    stateLayout->addWidget(m_handlerPosLabel, 3, 3);
    stateLayout->addWidget(new QLabel("Swap register", this), 4, 2);
    stateLayout->addWidget(m_swapLabel, 4, 3);

    leftCol->addWidget(stateBox);

    auto* errorBox = new QGroupBox("Error injection", this);
    auto* errorLayout = new QHBoxLayout(errorBox);
    m_errorCombo = new QComboBox(this);
    for (auto it = m_errorDescriptions.constBegin(); it != m_errorDescriptions.constEnd(); ++it)
        m_errorCombo->addItem(it.value(), it.key());
    m_injectErrorButton = new QPushButton("Inject error", this);
    m_clearErrorButton = new QPushButton("Clear error", this);
    errorLayout->addWidget(m_errorCombo, 1);
    errorLayout->addWidget(m_injectErrorButton);
    errorLayout->addWidget(m_clearErrorButton);
    leftCol->addWidget(errorBox);

    auto* busyBox = new QGroupBox("Busy simulation", this);
    auto* busyLayout = new QHBoxLayout(busyBox);

    m_forceBusyCheck = new QCheckBox("Force busy", this);
    m_busy2sButton = new QPushButton("Busy 2s", this);
    m_busy5sButton = new QPushButton("Busy 5s", this);

    busyLayout->addWidget(m_forceBusyCheck);
    busyLayout->addWidget(m_busy2sButton);
    busyLayout->addWidget(m_busy5sButton);
    busyLayout->addStretch();

    leftCol->addWidget(busyBox);

    auto* swapBox = new QGroupBox("Swap station", this);
    auto* swapLayout = new QFormLayout(swapBox);
    m_swapFrontCombo = new QComboBox(this);
    m_swapFrontCombo->addItem("Front plate 1", 1);
    m_swapFrontCombo->addItem("Front plate 2", 2);
    m_swapPlate1Check = new QCheckBox("Plate 1 occupied", this);
    m_swapPlate2Check = new QCheckBox("Plate 2 occupied", this);
    swapLayout->addRow("Front side", m_swapFrontCombo);
    swapLayout->addRow(QString(), m_swapPlate1Check);
    swapLayout->addRow(QString(), m_swapPlate2Check);
    leftCol->addWidget(swapBox);

    auto* transferBox = new QGroupBox("Transfer station", this);
    auto* transferLayout = new QHBoxLayout(transferBox);

    m_transferManualCheck = new QCheckBox("Occupied", this);
    m_loadTransferButton = new QPushButton("Load", this);
    m_clearTransferButton = new QPushButton("Clear", this);

    transferLayout->addWidget(m_transferManualCheck);
    transferLayout->addWidget(m_loadTransferButton);
    transferLayout->addWidget(m_clearTransferButton);
    transferLayout->addStretch();

    leftCol->addWidget(transferBox);

    auto* manualBox = new QGroupBox("Local test command", this);
    auto* manualLayout = new QHBoxLayout(manualBox);
    m_localCommandEdit = new QLineEdit(this);
    m_localCommandEdit->setPlaceholderText("Example: ch:bs or mv:sw 001");
    m_localCommandSendButton = new QPushButton("Send", this);
    manualLayout->addWidget(m_localCommandEdit, 1);
    manualLayout->addWidget(m_localCommandSendButton);
    leftCol->addWidget(manualBox);
    leftCol->addStretch();

    auto* locationsBox = new QGroupBox("Locations (double click to toggle)", this);
    auto* locationsLayout = new QVBoxLayout(locationsBox);
    m_locationsTable = new QTableWidget(this);
    m_locationsTable->setRowCount(20);
    m_locationsTable->setColumnCount(10);
    m_locationsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_locationsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_locationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    locationsLayout->addWidget(m_locationsTable);
    rightCol->addWidget(locationsBox, 3);

    auto* logBox = new QGroupBox("RX / TX log", this);
    auto* logLayout = new QVBoxLayout(logBox);
    m_logEdit = new QPlainTextEdit(this);
    m_logEdit->setReadOnly(true);
    m_clearLogButton = new QPushButton("Clear log", this);
    logLayout->addWidget(m_logEdit, 1);
    logLayout->addWidget(m_clearLogButton, 0, Qt::AlignRight);
    rightCol->addWidget(logBox, 2);

    mainLayout->addLayout(middleLayout, 1);

    connect(m_refreshPortsButton, &QPushButton::clicked,
            this, &MainWindow::refreshPorts);
    connect(m_openPortButton, &QPushButton::clicked,
            this, &MainWindow::openSelectedPort);
    connect(m_closePortButton, &QPushButton::clicked,
            this, &MainWindow::closePort);
    connect(m_injectErrorButton, &QPushButton::clicked,
            this, &MainWindow::injectSelectedError);
    connect(m_clearErrorButton, &QPushButton::clicked,
            this, &MainWindow::clearInjectedError);
    connect(m_locationsTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::onLocationCellDoubleClicked);
    connect(m_swapFrontCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSwapSettingsChanged);
    connect(m_swapPlate1Check, &QCheckBox::toggled,
            this, &MainWindow::onSwapSettingsChanged);
    connect(m_swapPlate2Check, &QCheckBox::toggled,
            this, &MainWindow::onSwapSettingsChanged);
    connect(m_clearLogButton, &QPushButton::clicked,
            this, &MainWindow::clearLog);
    connect(m_localCommandSendButton, &QPushButton::clicked,
            this, &MainWindow::sendLocalCommand);
    connect(m_localCommandEdit, &QLineEdit::returnPressed,
            this, &MainWindow::sendLocalCommand);
    connect(m_forceBusyCheck, &QCheckBox::toggled,
            this, &MainWindow::setForcedBusy);

    connect(m_busy2sButton, &QPushButton::clicked,
            this, &MainWindow::simulateBusy2s);

    connect(m_busy5sButton, &QPushButton::clicked,
            this, &MainWindow::simulateBusy5s);

    connect(m_transferManualCheck, &QCheckBox::toggled,
            this, &MainWindow::onTransferManualChanged);

    connect(m_loadTransferButton, &QPushButton::clicked,
            this, &MainWindow::onLoadTransferClicked);

    connect(m_clearTransferButton, &QPushButton::clicked,
            this, &MainWindow::onClearTransferClicked);

}


void MainWindow::onTransferManualChanged(bool checked)
{
    m_engine.setTransferOccupied(checked);
}

void MainWindow::onLoadTransferClicked()
{

    m_engine.setTransferOccupied(true);
}

void MainWindow::onClearTransferClicked()
{

    m_engine.setTransferOccupied(false);
}

void MainWindow::refreshPorts()
{
    const QString current = m_portCombo->currentText();
    m_portCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : ports)
        m_portCombo->addItem(info.portName());

    const int idx = m_portCombo->findText(current);
    if (idx >= 0)
        m_portCombo->setCurrentIndex(idx);
}

void MainWindow::openSelectedPort()
{
    if (m_portCombo->currentText().isEmpty())
    {
        appendLog("SYS", "No serial port selected");
        return;
    }

    m_server.open(m_portCombo->currentText());
}

void MainWindow::closePort()
{
    m_server.close();
}

void MainWindow::updateFromState()
{
    const CytomatState& s = m_engine.state();

    m_busyCheck->setChecked(s.forcedBusy || s.motionBusy);
    m_readyCheck->setChecked(s.ready);
    m_warningCheck->setChecked(s.warning);
    m_errorCheck->setChecked(s.error);
    m_handlerCheck->setChecked(s.handlerOccupied);
    m_transferCheck->setChecked(s.transferOccupied);
    m_gateCheck->setChecked(s.gateOpen);
    m_doorCheck->setChecked(s.deviceDoorOpen);

    m_warningCodeLabel->setText(QString("%1").arg(s.warningCode, 2, 16, QChar('0')));
    m_errorCodeLabel->setText(QString("%1").arg(s.errorCode, 2, 16, QChar('0')));
    m_actionCodeLabel->setText(QString("%1").arg(s.actionCode, 2, 16, QChar('0')));
    m_handlerPosLabel->setText(handlerPosToString(s.handlerPos));

    const QString swapText = QString("%1%2%3")
            .arg(s.swapFrontPlate)
            .arg(s.swapPlate1Occupied ? 1 : 0)
            .arg(s.swapPlate2Occupied ? 1 : 0);
    m_swapLabel->setText(swapText);

    m_swapFrontCombo->blockSignals(true);
    m_swapPlate1Check->blockSignals(true);
    m_swapPlate2Check->blockSignals(true);
    m_swapFrontCombo->setCurrentIndex(s.swapFrontPlate == 2 ? 1 : 0);
    m_swapPlate1Check->setChecked(s.swapPlate1Occupied);
    m_swapPlate2Check->setChecked(s.swapPlate2Occupied);
    m_swapFrontCombo->blockSignals(false);
    m_swapPlate1Check->blockSignals(false);
    m_swapPlate2Check->blockSignals(false);

    for (int loc = 1; loc <= 200; ++loc)
        updateLocationCell(loc);
}

void MainWindow::injectSelectedError()
{
    const quint8 code = static_cast<quint8>(m_errorCombo->currentData().toInt());
    m_engine.injectError(code);
    appendLog("SYS", QString("Injected error %1").arg(code, 2, 16, QChar('0')));
}

void MainWindow::clearInjectedError()
{
    m_engine.clearError();
    appendLog("SYS", "Cleared error register");
}

void MainWindow::onLocationCellDoubleClicked(int row, int column)
{
    const int loc = row * 10 + column + 1;
    const bool newValue = !m_engine.state().locations[loc];
    m_engine.setLocationOccupied(loc, newValue);
    appendLog("SYS", QString("Location %1 -> %2")
                    .arg(loc, 3, 10, QChar('0'))
                    .arg(newValue ? "occupied" : "empty"));
}

void MainWindow::onSwapSettingsChanged()
{
    m_engine.setSwapState(m_swapFrontCombo->currentData().toInt(),
                          m_swapPlate1Check->isChecked(),
                          m_swapPlate2Check->isChecked());
}

void MainWindow::clearLog()
{
    m_logEdit->clear();
}

void MainWindow::sendLocalCommand()
{
    const QString cmd = m_localCommandEdit->text().trimmed();
    if (cmd.isEmpty())
        return;

    appendLog("LOCAL RX", cmd);
    const QString reply = m_engine.handleLine(cmd);
    appendLog("LOCAL TX", reply.trimmed());
    m_localCommandEdit->clear();
}

void MainWindow::appendRx(const QString& line)
{
    appendLog("RX", line);
}

void MainWindow::appendTx(const QString& line)
{
    appendLog("TX", line);
}

void MainWindow::onPortOpened(const QString& name)
{
    m_portStatusLabel->setText(QString("Open: %1").arg(name));
    appendLog("SYS", QString("Port opened: %1").arg(name));
}

void MainWindow::onPortClosed()
{
    m_portStatusLabel->setText("Port closed");
    appendLog("SYS", "Port closed");
}

void MainWindow::onPortError(const QString& msg)
{
    appendLog("ERR", msg);
}

void MainWindow::appendLog(const QString& prefix, const QString& text)
{
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    m_logEdit->appendPlainText(QString("[%1] %2: %3").arg(ts, prefix, text));
}

void MainWindow::populateLocationsTable()
{
    QStringList headers;
    for (int c = 0; c < 10; ++c)
        headers << QString::number(c + 1);
    m_locationsTable->setHorizontalHeaderLabels(headers);

    QStringList rows;
    for (int r = 0; r < 20; ++r)
        rows << QString::number(r + 1);
    m_locationsTable->setVerticalHeaderLabels(rows);

    for (int loc = 1; loc <= 200; ++loc)
    {
        const int row = (loc - 1) / 10;
        const int col = (loc - 1) % 10;
        auto* item = new QTableWidgetItem(QString("%1").arg(loc, 3, 10, QChar('0')));
        item->setTextAlignment(Qt::AlignCenter);
        m_locationsTable->setItem(row, col, item);
    }
}

void MainWindow::updateLocationCell(int location)
{
    if (location < 1 || location > 200)
        return;

    const int row = (location - 1) / 10;
    const int col = (location - 1) % 10;
    QTableWidgetItem* item = m_locationsTable->item(row, col);
    if (!item)
        return;

    const bool occupied = m_engine.state().locations[location];
    item->setBackground(occupied ? QColor(198, 239, 206) : QColor(244, 204, 204));
    item->setToolTip(QString("Location %1 is %2")
                     .arg(location, 3, 10, QChar('0'))
                     .arg(occupied ? "occupied" : "empty"));
}

QString MainWindow::handlerPosToString(HandlerPos pos) const
{
    switch (pos)
    {
    case HandlerPos::Init: return "Init";
    case HandlerPos::Wait: return "Wait";
    case HandlerPos::Transfer: return "Transfer";
    case HandlerPos::Stacker: return "Stacker";
    case HandlerPos::Exposed: return "Exposed";
    }
    return "Unknown";
}


void MainWindow::setForcedBusy(bool checked)
{
        m_engine.setBusyForced(checked);
}

void MainWindow::simulateBusy2s()
{
        m_engine.simulateBusy(2000);
}

void MainWindow::simulateBusy5s()
{
        m_engine.simulateBusy(5000);
}