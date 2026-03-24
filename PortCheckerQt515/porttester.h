#pragma once

#include <QObject>
#include <QHostAddress>
#include <QAbstractSocket>

class QTcpSocket;

class PortTester : public QObject
{
    Q_OBJECT
public:
    enum class Mode { LocalListen, RemoteConnect };
    enum class AddressFamily { Any, IPv4Only, IPv6Only };

    explicit PortTester(QObject *parent = nullptr);

    void testLocal(quint16 port, AddressFamily fam);
    void testRemote(const QString &host, quint16 port, int timeoutMs);
    void cancel();

signals:
    void started(Mode mode);
    void finished(bool ok, const QString &summary, const QString &details);

private:
    void finish(bool ok, const QString &summary, const QString &details);

    QTcpSocket *m_socket = nullptr;
    bool m_running = false;
};
