#include "serial_server.h"
#include "cytomat_engine.h"

SerialServer::SerialServer(CytomatEngine* engine, QObject* parent)
    : QObject(parent), m_engine(engine)
{
    connect(&m_serial, &QSerialPort::readyRead,
            this, &SerialServer::onReadyRead);
}

bool SerialServer::open(const QString& portName, qint32 baudRate)
{
    if (m_serial.isOpen())
        m_serial.close();

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    const bool ok = m_serial.open(QIODevice::ReadWrite);
    if (!ok)
    {
        emit portError(QString("Cannot open %1: %2").arg(portName, m_serial.errorString()));
        return false;
    }

    emit portOpened(portName);
    return true;
}

void SerialServer::close()
{
    if (!m_serial.isOpen())
        return;
    m_serial.close();
    emit portClosed();
}

bool SerialServer::isOpen() const
{
    return m_serial.isOpen();
}

QString SerialServer::portName() const
{
    return m_serial.portName();
}

void SerialServer::onReadyRead()
{
    m_rxBuffer.append(m_serial.readAll());

    while (true)
    {
        const int idx = m_rxBuffer.indexOf('\r');
        if (idx < 0)
            break;

        const QByteArray line = m_rxBuffer.left(idx);
        m_rxBuffer.remove(0, idx + 1);

        const QString cmd = QString::fromLatin1(line).trimmed();
        emit rxLine(cmd);

        const QString reply = m_engine->handleLine(cmd);
        writeReply(reply);
    }
}

void SerialServer::writeReply(const QString& reply)
{
    emit txLine(reply.trimmed());

    if (!m_serial.isOpen())
        return;

    m_serial.write(reply.toLatin1());
    m_serial.flush();
}
