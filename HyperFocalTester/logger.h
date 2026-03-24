#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QMutex>

class Logger
{
public:
    static void install();

private:
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &msg);

    static QFile *m_logFile;
    static QMutex m_mutex;
};

#endif
