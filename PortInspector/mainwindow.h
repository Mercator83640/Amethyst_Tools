#pragma once

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnCheckLocal_clicked();
    void on_btnCheckRemote_clicked();
    void on_btnFullCheck_clicked();
    void on_btnClear_clicked();

private:
    Ui::MainWindow *ui;

    enum class VerdictMode { ServerLocalFree, ClientRemoteOpen, Both };

    void logLine(const QString& s);
    QString host() const;
    quint16 port() const;
    int timeoutMs() const;
    VerdictMode mode() const;

    bool isPortFreeLocal(quint16 port, QString& why);
    bool canConnectRemote(const QString& host, quint16 port, int timeoutMs, QString& why);

    void setVerdict(const QString& title, bool ok, const QString& details);
};
