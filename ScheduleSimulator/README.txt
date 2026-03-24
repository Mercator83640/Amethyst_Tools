Schedule Simulator (Qt Widgets)
================================

Role
----
This small Qt application simulates Schedule for Acquisition.

Network behavior
----------------
- Sends START_ANALYSIS JSON to Acquisition on port 10332
- Sends ACK_RELEASE JSON to Acquisition on port 10333
- Sends ABORT JSON to Acquisition on port 10332
- Listens for microscope status on port 12345
- When status 2 (RELEASE) is received, it can automatically send ACK_RELEASE

Default payload for START_ANALYSIS
----------------------------------
{
  "Cmd": "START_ANALYSIS",
  "CB": "<value from UI>",
  "Date": "<today ISO>",
  "DualAnalysis": false,
  "TsUtc": "<utc timestamp>"
}

Usage
-----
1. Open ScheduleSimulator.pro in Qt Creator
2. Build and run
3. Click "Demarrer"
4. Type a CB
5. Click "Envoyer START_ANALYSIS"
6. Watch status changes from Acquisition
7. If auto-ACK is enabled, ACK_RELEASE is sent when status becomes RELEASE

Notes
-----
- Host defaults to 127.0.0.1
- Ports default to 10332 / 10333 / 12345
- UI is intentionally simple for integration tests
