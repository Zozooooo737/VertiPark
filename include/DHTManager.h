#pragma once
#include <DHT.h>
#include "config.h"

class DHTManager {
public:
    DHTManager();
    void begin();
    float getTemperature();
    float getHumidity();
    bool isDataValid();

private:
    DHT _dht;
};