#include "Logger.h"

#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QDir>

QFile* Logger::m_logFile = nullptr;
QMutex Logger::m_mutex;

void Logger::install()
{
    QString logDir = "C:/adretek_log";

    QDir dir;
    if (!dir.exists(logDir))
        dir.mkpath(logDir);

    QString logFile =
        logDir + "/HyperFocalTester.log";

    m_logFile = new QFile(logFile);

    m_logFile->open(QIODevice::Append | QIODevice::Text);

    qInstallMessageHandler(Logger::messageHandler);

    qInfo() << "Logger started ->" << logFile;
}

void Logger::messageHandler(QtMsgType type,
                            const QMessageLogContext &context,
                            const QString &msg)
{
    QMutexLocker locker(&m_mutex);

    QString level;

    switch (type)
    {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "CRIT"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    QString time =
        QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString line =
        QString("%1 [%2] %3 (%4:%5)\n")
            .arg(time)
            .arg(level)
            .arg(msg)
            .arg(context.file ? context.file : "")
            .arg(context.line);

    if (m_logFile && m_logFile->isOpen())
    {
        QTextStream out(m_logFile);
        out << line;
        m_logFile->flush();
    }

    fprintf(stderr,"%s",line.toLocal8Bit().constData());
}
