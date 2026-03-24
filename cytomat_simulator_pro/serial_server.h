#pragma once

#include <QObject>
#include <QSerialPort>

class CytomatEngine;

class SerialServer : public QObject
{
    Q_OBJECT

public:
    explicit SerialServer(CytomatEngine* engine, QObject* parent = nullptr);

    bool open(const QString& portName, qint32 baudRate = QSerialPort::Baud9600);
    void close();
    bool isOpen() const;
    QString portName() const;

signals:
    void rxLine(const QString& line);
    void txLine(const QString& line);
    void portOpened(const QString& name);
    void portClosed();
    void portError(const QString& msg);

private slots:
    void onReadyRead();

private:
    void writeReply(const QString& reply);

private:
    QSerialPort m_serial;
    QByteArray m_rxBuffer;
    CytomatEngine* m_engine = nullptr;
};
