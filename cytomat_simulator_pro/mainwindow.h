#pragma once

#include <QMainWindow>
#include <QMap>

#include "cytomat_engine.h"
#include "serial_server.h"

class QComboBox;
class QPushButton;
class QLabel;
class QCheckBox;
class QTableWidget;
class QPlainTextEdit;
class QLineEdit;
class QSpinBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void refreshPorts();
    void openSelectedPort();
    void closePort();
    void updateFromState();
    void injectSelectedError();
    void clearInjectedError();
    void onLocationCellDoubleClicked(int row, int column);
    void onSwapSettingsChanged();
    void clearLog();
    void sendLocalCommand();
    void appendRx(const QString& line);
    void appendTx(const QString& line);
    void onPortOpened(const QString& name);
    void onPortClosed();
    void onPortError(const QString& msg);

private slots:
    void setForcedBusy(bool checked);
    void simulateBusy2s();
    void simulateBusy5s();
    void onTransferManualChanged(bool checked);
    void onLoadTransferClicked();
    void onClearTransferClicked();

private:
    void buildUi();
    void appendLog(const QString& prefix, const QString& text);
    void populateLocationsTable();
    void updateLocationCell(int location);

    QString handlerPosToString(HandlerPos pos) const;

private:
    CytomatEngine m_engine;
    SerialServer m_server;

    QComboBox* m_portCombo = nullptr;
    QPushButton* m_refreshPortsButton = nullptr;
    QPushButton* m_openPortButton = nullptr;
    QPushButton* m_closePortButton = nullptr;
    QLabel* m_portStatusLabel = nullptr;

    QCheckBox* m_forceBusyCheck = nullptr;
    QPushButton* m_busy2sButton = nullptr;
    QPushButton* m_busy5sButton = nullptr;

    QCheckBox* m_busyCheck = nullptr;
    QCheckBox* m_readyCheck = nullptr;
    QCheckBox* m_warningCheck = nullptr;
    QCheckBox* m_errorCheck = nullptr;
    QCheckBox* m_handlerCheck = nullptr;
    QCheckBox* m_transferCheck = nullptr;
    QCheckBox* m_gateCheck = nullptr;
    QCheckBox* m_doorCheck = nullptr;

    QLabel* m_warningCodeLabel = nullptr;
    QLabel* m_errorCodeLabel = nullptr;
    QLabel* m_actionCodeLabel = nullptr;
    QLabel* m_handlerPosLabel = nullptr;
    QLabel* m_swapLabel = nullptr;

    QCheckBox* m_transferManualCheck = nullptr;
    QPushButton* m_loadTransferButton = nullptr;
    QPushButton* m_clearTransferButton = nullptr;

    QComboBox* m_errorCombo = nullptr;
    QPushButton* m_injectErrorButton = nullptr;
    QPushButton* m_clearErrorButton = nullptr;

    QComboBox* m_swapFrontCombo = nullptr;
    QCheckBox* m_swapPlate1Check = nullptr;
    QCheckBox* m_swapPlate2Check = nullptr;

    QTableWidget* m_locationsTable = nullptr;
    QPlainTextEdit* m_logEdit = nullptr;
    QPushButton* m_clearLogButton = nullptr;

    QLineEdit* m_localCommandEdit = nullptr;
    QPushButton* m_localCommandSendButton = nullptr;

    QMap<int, QString> m_errorDescriptions;
};
