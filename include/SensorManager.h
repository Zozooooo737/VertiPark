#pragma once
#include <Arduino.h>
#include "Config.h"

class SensorManager {
public:
    SensorManager();

    void begin();

    bool isCarAtEntry();
    bool isCarAtExit();

    int getLight(uint8_t index);

    float getAirQuality();

    bool isHallTriggered();

private:
    int readMux(uint8_t channel);
};