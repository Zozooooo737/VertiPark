#pragma once
#include <Arduino.h>
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"

class HardwareTest {
private:
    RFIDManager& _rfid;
    SensorManager& _sensors;
    GateServoManager& _gate;
    PivotServoManager& _pivot;

public:
    HardwareTest(RFIDManager& rfid, SensorManager& sensors, GateServoManager& gate, PivotServoManager& pivot);
    
    void runRFIDTest();
    void runIRTest();
    void runGateTest();
    void runPivotTest();

    void runFullDiagnostic();
};