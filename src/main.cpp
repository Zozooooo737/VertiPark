#include <Arduino.h>
#include "SensorManager.h"
#include "GateServoManager.h"
#include "SystemManager.h"

SensorManager sensorManager;
GateServoManager gateManager;

SystemManager systemManager(sensorManager, gateManager);

void setup() {
    Serial.begin(115200);
    
    sensorManager.begin();
    gateManager.begin();

    systemManager.begin();
}

void loop() {
    systemManager.process();
    delay(500);
}