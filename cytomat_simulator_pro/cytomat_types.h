#pragma once

#include <QString>
#include <QVector>
#include <functional>

enum class HandlerPos
{
    Init,
    Wait,
    Transfer,
    Stacker,
    Exposed
};

enum class CommandType
{
    Unknown,

    ChBs,
    ChBe,
    ChBa,
    ChBw,
    ChSw,
    RsBe,

    MvTs,
    MvSt,
    MvSw,
    MvWs,
    MvWt,
    MvTw,
    MvSh,
    MvHs
};

struct ParsedCommand
{
    CommandType type = CommandType::Unknown;
    int location = -1;
    bool hasLocation = false;
    QString raw;
};

struct MotionStep
{
    int delayMs = 0;
    std::function<void()> apply;
};

struct CytomatState
{
    bool forcedBusy = false;
    bool motionBusy = false;

    bool isBusy() const
    {
        return forcedBusy || motionBusy;
    }


    bool ready = false;
    bool warning = false;
    bool error = false;

    bool handlerOccupied = false;
    bool transferOccupied = false;
    bool gateOpen = false;
    bool deviceDoorOpen = false;

    quint8 warningCode = 0x00;
    quint8 errorCode = 0x00;
    quint8 actionCode = 0x00;

    HandlerPos handlerPos = HandlerPos::Wait;

    int maxLocation = 200;
    QVector<bool> locations;

    int swapFrontPlate = 1;
    bool swapPlate1Occupied = false;
    bool swapPlate2Occupied = false;

    bool readyLatchedUntilNextBsRead = false;

    CytomatState()
    {
        locations.resize(maxLocation + 1);
        for (int i = 0; i < locations.size(); ++i)
            locations[i] = false;
    }
};
