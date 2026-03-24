#pragma once

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <QList>
#include "cytomat_types.h"

class CytomatEngine : public QObject
{
    Q_OBJECT

public:
    explicit CytomatEngine(QObject* parent = nullptr);

    QString handleLine(const QString& line);

    const CytomatState& state() const { return m_state; }

    void setBusyForced(bool busy);
    void injectError(quint8 code);
    void clearError();
    void setLocationOccupied(int location, bool occupied);
    void setSwapState(int frontPlate, bool plate1Occupied, bool plate2Occupied);
    void resetDemoState();
    void simulateBusy(int durationMs);
    bool isBusy() const;
    void setTransferOccupied(bool occupied);

signals:
    void stateChanged();
    void traceMessage(const QString& msg);

private slots:
    void runNextMotionStep();

private:
    ParsedCommand parseCommand(const QString& line) const;

    QString handleQuery(const ParsedCommand& cmd);
    QString handleMotionCommand(const ParsedCommand& cmd);

    QString makeOkReply() const;
    QString makeErrorReply(quint8 code) const;
    QString makeBsReply();
    QString makeBeReply() const;
    QString makeBaReply() const;
    QString makeBwReply() const;
    QString makeSwReply() const;

    quint8 makeOverviewByte() const;
    bool isValidLocation(int location) const;

    quint8 validateMotion(const ParsedCommand& cmd) const;

    void startMotion(const QList<MotionStep>& steps, quint8 actionCode);
    void finishMotion();

    QList<MotionStep> buildMvSt(int loc);
    QList<MotionStep> buildMvTs(int loc);
    QList<MotionStep> buildMvSw(int loc);
    QList<MotionStep> buildMvWs(int loc);
    QList<MotionStep> buildMvWt();
    QList<MotionStep> buildMvTw();
    QList<MotionStep> buildMvSh(int loc);
    QList<MotionStep> buildMvHs(int loc);

    void setBusy(quint8 actionCode);
    void setErrorInternal(quint8 code);

private:
    CytomatState m_state;
    QQueue<MotionStep> m_motionQueue;
    QTimer m_motionTimer;
};
