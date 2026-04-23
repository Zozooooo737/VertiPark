#pragma once
#include <Arduino.h>
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "DisplayManager.h"
#include "BuzzerManager.h"

class HardwareTest {
private:
    RFIDManager& _rfid;
    SensorManager& _sensors;
    GateServoManager& _gate;
    PivotServoManager& _pivot;
    DisplayManager& _display;
    BuzzerManager& _buzzer;

public:
    HardwareTest(RFIDManager& rfid, SensorManager& sensors, GateServoManager& gate, PivotServoManager& pivot, DisplayManager& display, BuzzerManager& buzzer);
    
    void runRFIDTest();
    void runIRTest();
    void runGateTest();
    void runPivotTest();
    void runDisplayTest();
    void runBuzzerTest();

    void runFullDiagnostic();
};