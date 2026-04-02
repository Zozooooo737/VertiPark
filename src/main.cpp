#include <Arduino.h>
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "SystemManager.h"

SensorManager sensorManager;
GateServoManager gateManager;
PivotServoManager pivotManager;

SystemManager systemManager(sensorManager, gateManager, pivotManager);


void setup() {
    Serial.begin(115200);
    
    sensorManager.begin();
    gateManager.begin();
    pivotManager.begin();

    systemManager.begin();
}

void loop() {
    systemManager.process();
    delay(500);
}