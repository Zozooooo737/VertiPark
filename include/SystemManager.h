#pragma once
#include <Arduino.h>
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"

class SystemManager {
public:
    SystemManager(SensorManager& sensorMgr,
              GateServoManager& gateMgr,
              PivotServoManager& pivotMgr);

    void begin();
    void process();

private:
    SensorManager& sensorManager;
    GateServoManager& gateManager;
    PivotServoManager& pivotManager;
};