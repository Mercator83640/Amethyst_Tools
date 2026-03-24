#pragma once

#include <QMainWindow>

class QLineEdit;
class QSpinBox;
class QComboBox;
class QPushButton;
class QLabel;
class QTextEdit;
class QTabWidget;
class PortTester;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void appendLog(const QString &text);

private slots:
    void onTestLocal();
    void onTestRemote();
    void onCancel();
    void onStarted(int mode);
    void onFinished(bool ok, const QString &summary, const QString &details);

private:
    // Widgets
    QLineEdit *m_hostEdit = nullptr;
    QSpinBox *m_portSpin = nullptr;
    QComboBox *m_familyCombo = nullptr;
    QSpinBox *m_timeoutSpin = nullptr;

    QPushButton *m_btnLocal = nullptr;
    QPushButton *m_btnRemote = nullptr;
    QPushButton *m_btnCancel = nullptr;

    QLabel *m_statusLabel = nullptr;
    QTextEdit *m_details = nullptr;
    QTextEdit *m_log = nullptr;

    PortTester *m_tester = nullptr;
};
