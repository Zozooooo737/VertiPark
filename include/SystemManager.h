#pragma once
#include <Arduino.h>
#include "SensorManager.h"
#include "GateServoManager.h"

class SystemManager {
public:
    SystemManager(SensorManager& sensorMgr, GateServoManager& gateMgr);

    void begin();
    void process();

private:
    SensorManager& sensorManager;
    GateServoManager& gateManager;
};