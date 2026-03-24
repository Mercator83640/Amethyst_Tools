# CytomatSimulator

Qt Widgets simulator for a Thermo Fisher Cytomat-like serial interface.

## Features

- Serial listener on a virtual COM port using `QSerialPort`
- Immediate replies terminated by `\r` for synchronous host software
- Simulated internal motion steps with timers
- Supported commands:
  - `ch:bs`
  - `ch:be`
  - `ch:bw`
  - `ch:ba`
  - `ch:sw`
  - `rs:be`
  - `mv:ts ###`
  - `mv:st ###`
  - `mv:sw ###`
  - `mv:ws ###`
  - `mv:wt`
  - `mv:tw`
  - `mv:sh ###`
  - `mv:hs ###`
- UI with:
  - live state display
  - error injection
  - 200 location occupancy display
  - swap station editing
  - RX/TX log
  - local command test box

## Build

Open `CytomatSimulator.pro` with Qt Creator and build.

Required modules:

- Qt Widgets
- Qt SerialPort

## Notes

This is a functional simulator skeleton for integration and UI testing.
You can tune timings, action codes and error mapping in `cytomat_engine.cpp`.
