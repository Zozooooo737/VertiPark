#include <Arduino.h>
#include "config.h"
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "HardwareTest.h"

RFIDManager rfid;
SensorManager sensors;
GateServoManager gate;
PivotServoManager pivot;

HardwareTest tester(rfid, sensors, gate, pivot);

void setup() {
    Serial.begin(115200);

    sensors.begin();
    rfid.begin();
    gate.begin();
    pivot.begin();
}

void loop() {
    tester.runFullDiagnostic();
    
    delay(2000);
}