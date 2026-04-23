#pragma once
#include <Arduino.h>
#include "config.h"

class BuzzerManager {
public:
    BuzzerManager();
    void begin();
    void playSuccess();
    void playError();
    void playAlert();
    void stop();

private:
    void beep(int frequency, int duration);
};