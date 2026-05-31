#pragma once
#include <Arduino.h>
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "DisplayManager.h"
#include "BuzzerManager.h"
#include "DHTManager.h"
#include "LedsManager.h"

class HardwareTest {
private:
    RFIDManager& _rfid;
    SensorManager& _sensors;
    GateServoManager& _gate;
    PivotServoManager& _pivot;
    DisplayManager& _display;
    BuzzerManager& _buzzer;
    DHTManager& _temp;
    LedsManager& _leds;

public:
    HardwareTest(RFIDManager& rfid, SensorManager& sensors, GateServoManager& gate, PivotServoManager& pivot, DisplayManager& display, BuzzerManager& buzzer, DHTManager& temp, LedsManager& leds);
    
    void runRFIDTest();
    void runIRTest();
    void runHallTest();
    void runGateTest();
    void runPivotTest();
    void runDisplayTest();
    void runBuzzerTest();
    void runDHTTest();
    void runLedsTest();

    void runFullDiagnostic();
};