#pragma once
#include <Arduino.h>
#include "Config.h"

class SensorManager {
public:
    SensorManager();

    void begin();

    bool isCarAtEntry();
    bool isCarAtExit();
    
    float getAirQuality();
    bool isHallTriggered();

private:
    int readMuxChannel(uint8_t channel);
};