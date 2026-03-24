#include "cytomat_engine.h"

#include <QRegularExpression>
#include <QDebug>

namespace
{
constexpr quint8 ERR_BUSY              = 0x01;
constexpr quint8 ERR_UNKNOWN_COMMAND   = 0x02;
constexpr quint8 ERR_TELEGRAM          = 0x03;
constexpr quint8 ERR_PARAM             = 0x04;
constexpr quint8 ERR_UNKNOWN_LOCATION  = 0x05;
constexpr quint8 ERR_HANDLER_OCCUPIED  = 0x21;
constexpr quint8 ERR_HANDLER_EMPTY     = 0x22;
constexpr quint8 ERR_TRANSFER_EMPTY    = 0x31;
constexpr quint8 ERR_TRANSFER_OCCUPIED = 0x32;

constexpr quint8 ACT_IDLE           = 0x00;
constexpr quint8 ACT_MOVE_STACKER   = 0x10;
constexpr quint8 ACT_MOVE_TRANSFER  = 0x20;
constexpr quint8 ACT_MOVE_WAIT      = 0x30;
constexpr quint8 ACT_MOVE_EXPOSED   = 0x40;
constexpr quint8 ACT_OPEN_GATE      = 0x50;
constexpr quint8 ACT_CLOSE_GATE     = 0x51;
constexpr quint8 ACT_EXTEND_SHOVEL  = 0x60;
constexpr quint8 ACT_RETRACT_SHOVEL = 0x61;
}

CytomatEngine::CytomatEngine(QObject* parent)
    : QObject(parent)
{
    connect(&m_motionTimer, &QTimer::timeout,
            this, &CytomatEngine::runNextMotionStep);
    m_motionTimer.setSingleShot(true);
    resetDemoState();
}

void CytomatEngine::resetDemoState()
{
    m_motionQueue.clear();
    m_motionTimer.stop();
    m_state = CytomatState();
    for (int i = 1; i < m_state.locations.size(); ++i)
        m_state.locations[i] = false;

    m_state.transferOccupied = true;
    m_state.handlerOccupied = false;
    m_state.forcedBusy = false;
    m_state.motionBusy = false;
    m_state.ready = false;
    m_state.warning = false;
    m_state.error = false;
    m_state.gateOpen = false;
    m_state.deviceDoorOpen = false;
    m_state.actionCode = 0x00;
    m_state.warningCode = 0x00;
    m_state.errorCode = 0x00;
    m_state.handlerPos = HandlerPos::Wait;

    m_state.swapFrontPlate = 1;
    m_state.swapPlate1Occupied = false;
    m_state.swapPlate2Occupied = false;
    m_state.readyLatchedUntilNextBsRead = false;

    emit stateChanged();
}

void CytomatEngine::setBusyForced(bool busy)
{
    m_state.forcedBusy = busy;

    if (!isBusy())
        m_state.actionCode = 0x00;
    else if (m_state.actionCode == 0x00)
        m_state.actionCode = 0x10;

    emit stateChanged();
    QString msg = QString("ForceBusy=%1 effectiveBusy=%2")
                      .arg(busy ? "true" : "false")
                      .arg(isBusy() ? "true" : "false");

    qDebug() << msg;
}

void CytomatEngine::simulateBusy(int durationMs)
{
    setBusyForced(true);

    QTimer::singleShot(durationMs, this, [this]() {
        setBusyForced(false);
    });
}

QString CytomatEngine::handleLine(const QString& line)
{
    const ParsedCommand cmd = parseCommand(line);

    switch (cmd.type)
    {
    case CommandType::ChBs:
    case CommandType::ChBe:
    case CommandType::ChBa:
    case CommandType::ChBw:
    case CommandType::ChSw:
    case CommandType::RsBe:
        return handleQuery(cmd);

    case CommandType::MvTs:
    case CommandType::MvSt:
    case CommandType::MvSw:
    case CommandType::MvWs:
    case CommandType::MvWt:
    case CommandType::MvTw:
    case CommandType::MvSh:
    case CommandType::MvHs:
        return handleMotionCommand(cmd);

    default:
        return makeErrorReply(ERR_UNKNOWN_COMMAND);
    }
}

ParsedCommand CytomatEngine::parseCommand(const QString& line) const
{
    ParsedCommand cmd;
    cmd.raw = line.trimmed();

    const QString s = cmd.raw.toLower();

    if (s == "ch:bs") { cmd.type = CommandType::ChBs; return cmd; }
    if (s == "ch:be") { cmd.type = CommandType::ChBe; return cmd; }
    if (s == "ch:ba") { cmd.type = CommandType::ChBa; return cmd; }
    if (s == "ch:bw") { cmd.type = CommandType::ChBw; return cmd; }
    if (s == "ch:sw") { cmd.type = CommandType::ChSw; return cmd; }
    if (s == "rs:be") { cmd.type = CommandType::RsBe; return cmd; }

    auto parseLoc = [&](const QString& prefix, CommandType t) -> bool
    {
        QRegularExpression re("^" + QRegularExpression::escape(prefix) + "\\s*(\\d{3})$");
        const QRegularExpressionMatch m = re.match(s);
        if (!m.hasMatch())
            return false;

        cmd.type = t;
        cmd.location = m.captured(1).toInt();
        cmd.hasLocation = true;
        return true;
    };

    if (parseLoc("mv:ts", CommandType::MvTs)) return cmd;
    if (parseLoc("mv:st", CommandType::MvSt)) return cmd;
    if (parseLoc("mv:sw", CommandType::MvSw)) return cmd;
    if (parseLoc("mv:ws", CommandType::MvWs)) return cmd;
    if (parseLoc("mv:sh", CommandType::MvSh)) return cmd;
    if (parseLoc("mv:hs", CommandType::MvHs)) return cmd;

    if (s == "mv:wt") { cmd.type = CommandType::MvWt; return cmd; }
    if (s == "mv:tw") { cmd.type = CommandType::MvTw; return cmd; }

    return cmd;
}

QString CytomatEngine::handleQuery(const ParsedCommand& cmd)
{
    switch (cmd.type)
    {
    case CommandType::ChBs:
        return makeBsReply();
    case CommandType::ChBe:
        return makeBeReply();
    case CommandType::ChBa:
        return makeBaReply();
    case CommandType::ChBw:
        return makeBwReply();
    case CommandType::ChSw:
        return makeSwReply();
    case CommandType::RsBe:
        clearError();
        return makeOkReply();
    default:
        return makeErrorReply(ERR_UNKNOWN_COMMAND);
    }
}

QString CytomatEngine::handleMotionCommand(const ParsedCommand& cmd)
{
    const quint8 err = validateMotion(cmd);
    if (err != 0x00)
        return makeErrorReply(err);

    switch (cmd.type)
    {
    case CommandType::MvSt:
        startMotion(buildMvSt(cmd.location), ACT_MOVE_STACKER);
        break;
    case CommandType::MvTs:
        startMotion(buildMvTs(cmd.location), ACT_MOVE_STACKER);
        break;
    case CommandType::MvSw:
        startMotion(buildMvSw(cmd.location), ACT_MOVE_STACKER);
        break;
    case CommandType::MvWs:
        startMotion(buildMvWs(cmd.location), ACT_MOVE_STACKER);
        break;
    case CommandType::MvWt:
        startMotion(buildMvWt(), ACT_MOVE_TRANSFER);
        break;
    case CommandType::MvTw:
        startMotion(buildMvTw(), ACT_MOVE_TRANSFER);
        break;
    case CommandType::MvSh:
        startMotion(buildMvSh(cmd.location), ACT_MOVE_EXPOSED);
        break;
    case CommandType::MvHs:
        startMotion(buildMvHs(cmd.location), ACT_MOVE_EXPOSED);
        break;
    default:
        return makeErrorReply(ERR_UNKNOWN_COMMAND);
    }

    return makeOkReply();
}

quint8 CytomatEngine::makeOverviewByte() const
{
    quint8 v = 0x00;
    if (isBusy())                 v |= 0x01;
    if (m_state.ready)            v |= 0x02;
    if (m_state.warning)          v |= 0x04;
    if (m_state.error)            v |= 0x08;
    if (m_state.handlerOccupied)  v |= 0x10;
    if (m_state.gateOpen)         v |= 0x20;
    if (m_state.deviceDoorOpen)   v |= 0x40;
    if (m_state.transferOccupied) v |= 0x80;
    return v;
}

QString CytomatEngine::makeOkReply() const
{
    return QString("ok %1\r").arg(makeOverviewByte(), 2, 16, QChar('0'));
}

QString CytomatEngine::makeErrorReply(quint8 code) const
{
    return QString("er %1\r").arg(code, 2, 16, QChar('0'));
}

QString CytomatEngine::makeBsReply()
{
    const QString rep = QString("bs %1\r").arg(makeOverviewByte(), 2, 16, QChar('0'));

    if (m_state.readyLatchedUntilNextBsRead)
    {
        m_state.ready = false;
        m_state.readyLatchedUntilNextBsRead = false;
        emit stateChanged();
    }

    return rep;
}

QString CytomatEngine::makeBeReply() const
{
    return QString("be %1\r").arg(m_state.errorCode, 2, 16, QChar('0'));
}

QString CytomatEngine::makeBaReply() const
{
    return QString("ba %1\r").arg(m_state.actionCode, 2, 16, QChar('0'));
}

QString CytomatEngine::makeBwReply() const
{
    return QString("bw %1\r").arg(m_state.warningCode, 2, 16, QChar('0'));
}

QString CytomatEngine::makeSwReply() const
{
    const QChar c1 = (m_state.swapFrontPlate == 1 ? '1' : '2');
    const QChar c2 = (m_state.swapFrontPlate == 1
                      ? (m_state.swapPlate1Occupied ? '1' : '0')
                      : (m_state.swapPlate2Occupied ? '1' : '0'));
    const QChar c3 = (m_state.swapFrontPlate == 1
                      ? (m_state.swapPlate2Occupied ? '1' : '0')
                      : (m_state.swapPlate1Occupied ? '1' : '0'));

    return QString("sw %1%2%3\r").arg(c1, c2, c3);
}

bool CytomatEngine::isValidLocation(int location) const
{
    return location >= 1 && location < m_state.locations.size();
}

quint8 CytomatEngine::validateMotion(const ParsedCommand& cmd) const
{
    qDebug() << "validateMotion"
             << "cmd=" << cmd.raw
             << "loc=" << cmd.location
             << "transferOccupied=" << m_state.transferOccupied
             << "handlerOccupied=" << m_state.handlerOccupied
             << "locOccupied=" << (cmd.hasLocation && isValidLocation(cmd.location)
                                       ? m_state.locations[cmd.location]
                                       : false);

    if (isBusy())
        return ERR_BUSY;

    if (cmd.hasLocation && !isValidLocation(cmd.location))
        return ERR_UNKNOWN_LOCATION;

    switch (cmd.type)
    {
    case CommandType::MvSt:
        if (m_state.handlerOccupied)
            return ERR_HANDLER_OCCUPIED;
        if (m_state.transferOccupied)
            return ERR_TRANSFER_OCCUPIED;
        if (!m_state.locations[cmd.location])
            return ERR_UNKNOWN_LOCATION;
        return 0x00;

    case CommandType::MvTs:
        if (m_state.handlerOccupied)
            return ERR_HANDLER_OCCUPIED;
        if (!m_state.transferOccupied)
            return ERR_TRANSFER_EMPTY;
        return 0x00;

    case CommandType::MvSw:
        if (m_state.handlerOccupied)
            return ERR_HANDLER_OCCUPIED;
        if (!m_state.locations[cmd.location])
            return ERR_UNKNOWN_LOCATION;
        return 0x00;

    case CommandType::MvWs:
        if (!m_state.handlerOccupied)
            return ERR_HANDLER_EMPTY;
        return 0x00;

    case CommandType::MvWt:
        if (!m_state.handlerOccupied)
            return ERR_HANDLER_EMPTY;
        if (m_state.transferOccupied)
            return ERR_TRANSFER_OCCUPIED;
        return 0x00;

    case CommandType::MvTw:
        if (m_state.handlerOccupied)
            return ERR_HANDLER_OCCUPIED;
        if (!m_state.transferOccupied)
            return ERR_TRANSFER_EMPTY;
        return 0x00;

    case CommandType::MvSh:
        if (m_state.handlerOccupied)
            return ERR_HANDLER_OCCUPIED;
        if (!m_state.locations[cmd.location])
            return ERR_UNKNOWN_LOCATION;
        return 0x00;

    case CommandType::MvHs:
        if (!m_state.handlerOccupied)
            return ERR_HANDLER_EMPTY;
        return 0x00;

    default:
        return ERR_UNKNOWN_COMMAND;
    }
}

void CytomatEngine::setBusy(quint8 actionCode)
{
    m_state.motionBusy = true;
    m_state.ready = false;
    m_state.actionCode = actionCode;
    emit stateChanged();
}

void CytomatEngine::finishMotion()
{
    m_state.motionBusy = false;

    if (!isBusy())
        m_state.actionCode = 0x00;

    emit stateChanged();
}

void CytomatEngine::injectError(quint8 code)
{
    setErrorInternal(code);
}

void CytomatEngine::clearError()
{
    m_state.error = false;
    m_state.errorCode = 0x00;
    emit stateChanged();
}

void CytomatEngine::setErrorInternal(quint8 code)
{
    m_state.error = true;
    m_state.errorCode = code;
    emit stateChanged();
}

void CytomatEngine::setLocationOccupied(int location, bool occupied)
{
    if (!isValidLocation(location))
        return;
    m_state.locations[location] = occupied;
    emit stateChanged();
}

void CytomatEngine::setSwapState(int frontPlate, bool plate1Occupied, bool plate2Occupied)
{
    m_state.swapFrontPlate = (frontPlate == 2 ? 2 : 1);
    m_state.swapPlate1Occupied = plate1Occupied;
    m_state.swapPlate2Occupied = plate2Occupied;
    emit stateChanged();
}

void CytomatEngine::startMotion(const QList<MotionStep>& steps, quint8 actionCode)
{
    m_motionQueue.clear();
    for (const MotionStep& s : steps)
        m_motionQueue.enqueue(s);

    setBusy(actionCode);
    runNextMotionStep();
}

void CytomatEngine::runNextMotionStep()
{
    if (m_motionQueue.isEmpty())
    {
        finishMotion();
        return;
    }

    const MotionStep step = m_motionQueue.dequeue();
    if (step.apply)
        step.apply();

    emit stateChanged();

    if (m_motionQueue.isEmpty())
    {
        finishMotion();
        return;
    }

    m_motionTimer.start(step.delayMs);
}

QList<MotionStep> CytomatEngine::buildMvSt(int loc)
{
    return {
        {300, [this]() {
             m_state.actionCode = ACT_MOVE_STACKER;
         }},
        {300, [this]() {
             m_state.actionCode = ACT_EXTEND_SHOVEL;
         }},
        {300, [this, loc]() {
             // La plaque quitte le stockage
             m_state.locations[loc] = false;
             m_state.handlerOccupied = true;
             m_state.handlerPos = HandlerPos::Transfer;
         }},
        {400, [this]() {
             m_state.actionCode = ACT_OPEN_GATE;
             m_state.gateOpen = true;
         }},
        {400, [this]() {
             m_state.actionCode = ACT_MOVE_TRANSFER;
         }},
        {300, [this]() {
             // Dépose sur transfer station
             m_state.handlerOccupied = false;
             m_state.transferOccupied = true;
             m_state.ready = true;
             m_state.readyLatchedUntilNextBsRead = true;
             m_state.handlerPos = HandlerPos::Wait;
         }},
        {300, [this]() {
             m_state.gateOpen = false;
             m_state.actionCode = ACT_CLOSE_GATE;
         }}
    };
}

QList<MotionStep> CytomatEngine::buildMvTs(int loc)
{
    return {
        {300, [this]() {
             m_state.actionCode = ACT_OPEN_GATE;
             m_state.gateOpen = true;
         }},
        {400, [this]() {
             m_state.actionCode = ACT_MOVE_TRANSFER;
         }},
        {300, [this]() {
             // Prise depuis transfer station
             m_state.transferOccupied = false;
             m_state.handlerOccupied = true;
             m_state.handlerPos = HandlerPos::Stacker;
         }},
        {400, [this]() {
             m_state.actionCode = ACT_MOVE_STACKER;
         }},
        {300, [this]() {
             m_state.actionCode = ACT_EXTEND_SHOVEL;
         }},
        {300, [this, loc]() {
             // Dépose en stockage
             m_state.locations[loc] = true;
             m_state.handlerOccupied = false;
             m_state.handlerPos = HandlerPos::Wait;
         }},
        {300, [this]() {
             m_state.gateOpen = false;
             m_state.actionCode = ACT_CLOSE_GATE;
         }}
    };

}

void CytomatEngine::setTransferOccupied(bool occupied)
{
    m_state.transferOccupied = occupied;
    emit stateChanged();
}

QList<MotionStep> CytomatEngine::buildMvSw(int loc)
{
    return {
        {400, [this]() { m_state.actionCode = ACT_MOVE_STACKER; }},
        {300, [this]() { m_state.actionCode = ACT_EXTEND_SHOVEL; }},
        {300, [this, loc]() {
            m_state.locations[loc] = false;
            m_state.handlerOccupied = true;
            m_state.handlerPos = HandlerPos::Wait;
        }},
        {300, [this]() { m_state.actionCode = ACT_RETRACT_SHOVEL; }}
    };
}

QList<MotionStep> CytomatEngine::buildMvWs(int loc)
{
    return {
        {400, [this]() { m_state.actionCode = ACT_MOVE_STACKER; }},
        {300, [this]() { m_state.actionCode = ACT_EXTEND_SHOVEL; }},
        {300, [this, loc]() {
            m_state.locations[loc] = true;
            m_state.handlerOccupied = false;
            m_state.handlerPos = HandlerPos::Wait;
        }},
        {300, [this]() { m_state.actionCode = ACT_RETRACT_SHOVEL; }}
    };
}

QList<MotionStep> CytomatEngine::buildMvWt()
{
    return {
        {300, [this]() { m_state.actionCode = ACT_OPEN_GATE; m_state.gateOpen = true; }},
        {400, [this]() { m_state.actionCode = ACT_MOVE_TRANSFER; }},
        {300, [this]() {
            m_state.handlerOccupied = false;
            m_state.transferOccupied = true;
            m_state.ready = true;
            m_state.readyLatchedUntilNextBsRead = true;
            m_state.handlerPos = HandlerPos::Wait;
        }},
        {300, [this]() { m_state.gateOpen = false; m_state.actionCode = ACT_CLOSE_GATE; }}
    };
}

QList<MotionStep> CytomatEngine::buildMvTw()
{
    return {
        {300, [this]() { m_state.actionCode = ACT_OPEN_GATE; m_state.gateOpen = true; }},
        {400, [this]() { m_state.actionCode = ACT_MOVE_TRANSFER; }},
        {300, [this]() {
            m_state.transferOccupied = false;
            m_state.handlerOccupied = true;
            m_state.handlerPos = HandlerPos::Wait;
        }},
        {300, [this]() { m_state.gateOpen = false; m_state.actionCode = ACT_CLOSE_GATE; }}
    };
}

QList<MotionStep> CytomatEngine::buildMvSh(int loc)
{
    return {
        {400, [this]() { m_state.actionCode = ACT_MOVE_STACKER; }},
        {300, [this]() { m_state.actionCode = ACT_EXTEND_SHOVEL; }},
        {300, [this, loc]() {
            m_state.locations[loc] = false;
            m_state.handlerOccupied = true;
        }},
        {500, [this]() {
            m_state.actionCode = ACT_MOVE_EXPOSED;
            m_state.handlerPos = HandlerPos::Exposed;
        }}
    };
}

QList<MotionStep> CytomatEngine::buildMvHs(int loc)
{
    return {
        {500, [this]() { m_state.actionCode = ACT_MOVE_STACKER; }},
        {300, [this]() { m_state.actionCode = ACT_EXTEND_SHOVEL; }},
        {300, [this, loc]() {
            m_state.locations[loc] = true;
            m_state.handlerOccupied = false;
            m_state.handlerPos = HandlerPos::Wait;
        }},
        {300, [this]() { m_state.actionCode = ACT_RETRACT_SHOVEL; }}
    };
}

bool CytomatEngine::isBusy() const
{
    return m_state.forcedBusy || m_state.motionBusy;
}