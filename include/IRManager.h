#pragma once
#include <Arduino.h>
#include "Config.h"

class IRManager {
public:
    IRManager();
    void begin();
    bool isCarAtEntry();
    bool isCarAtExit();
private:
    uint8_t entryPin;
    uint8_t exitPin;
};