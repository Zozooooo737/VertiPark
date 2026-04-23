#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "Config.h"

class PivotServoManager {
public:
    PivotServoManager();

    void begin();

    void rotateCW();
    void rotateCCW();
    void stop();
    void detach();

private:
    Servo pivotServo;
    int currentSpeed;
};