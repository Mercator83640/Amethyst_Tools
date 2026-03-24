#include "porttester.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

PortTester::PortTester(QObject *parent) : QObject(parent)
{
}

void PortTester::finish(bool ok, const QString &summary, const QString &details)
{
    m_running = false;
    emit finished(ok, summary, details);
}

void PortTester::cancel()
{
    if (!m_running)
        return;

    if (m_socket) {
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
    }

    m_running = false;
    emit finished(false, tr("Annulé"), tr("Le test a été annulé par l'utilisateur."));
}

void PortTester::testLocal(quint16 port, AddressFamily fam)
{
    if (m_running)
        cancel();

    m_running = true;
    emit started(Mode::LocalListen);

    QHostAddress address;
    switch (fam) {
    case AddressFamily::IPv4Only:
        address = QHostAddress(QHostAddress::AnyIPv4);
        break;
    case AddressFamily::IPv6Only:
        address = QHostAddress(QHostAddress::AnyIPv6);
        break;
    case AddressFamily::Any:
    default:
        address = QHostAddress(QHostAddress::Any);
        break;
    }

    QTcpServer server;
    const bool ok = server.listen(address, port);

    if (ok) {
        const auto bound = QString("%1:%2").arg(server.serverAddress().toString()).arg(server.serverPort());
        server.close();
        finish(true,
               tr("PORT LIBRE (local)"),
               tr("Le port %1 est disponible pour un serveur TCP local (bind/listen réussi).\nAdresse testée : %2\n\n"
                  "Remarque : ce test vérifie que le port n'est pas déjà utilisé sur cette machine.")
                   .arg(port)
                   .arg(bound));
    } else {
        const auto err = server.errorString();
        finish(false,
               tr("PORT DÉJÀ UTILISÉ (local)"),
               tr("Impossible d'écouter sur le port %1 : %2\n\n"
                  "Cela indique généralement que le port est déjà occupé ou qu'une règle système empêche l'écoute.")
                   .arg(port)
                   .arg(err));
    }
}

void PortTester::testRemote(const QString &host, quint16 port, int timeoutMs)
{
    if (m_running)
        cancel();

    m_running = true;
    emit started(Mode::RemoteConnect);

    m_socket = new QTcpSocket(this);

    // Timeout
    QTimer *timer = new QTimer(m_socket);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [this, host, port, timeoutMs]() {
        if (!m_socket)
            return;
        const QString msg = tr("Délai dépassé (%1 ms) en tentant de se connecter à %2:%3.\n"
                               "Interprétation : le port est probablement fermé, filtré (firewall) ou l'hôte est injoignable.")
                               .arg(timeoutMs)
                               .arg(host)
                               .arg(port);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
        finish(false, tr("PORT NON OUVERT (distance)"), msg);
    });

    connect(m_socket, &QTcpSocket::connected, this, [this, host, port]() {
        if (!m_socket)
            return;
        const QString msg = tr("Connexion réussie vers %1:%2.\n"
                               "Interprétation : un service écoute sur ce port (donc il n'est pas \"libre\" sur l'hôte distant).\n\n"
                               "Remarque importante : depuis un poste client, on ne peut pas prouver qu'un port est \"libre\" à distance ; "
                               "on peut seulement vérifier s'il est ouvert (service à l'écoute) ou non (refus/timeout).")
                               .arg(host)
                               .arg(port);
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
        finish(true, tr("PORT OUVERT (distance)"), msg);
    });

    connect(m_socket, &QTcpSocket::errorOccurred, this, [this, host, port](QAbstractSocket::SocketError) {
        if (!m_socket)
            return;
        const QString err = m_socket->errorString();
        const QString msg = tr("Échec de connexion vers %1:%2 : %3\n"
                               "Interprétation : le port est fermé, filtré (firewall), ou l'hôte/nom n'est pas joignable.")
                               .arg(host)
                               .arg(port)
                               .arg(err);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = nullptr;
        finish(false, tr("PORT NON OUVERT (distance)"), msg);
    });

    timer->start(timeoutMs);
    m_socket->connectToHost(host, port);
}
