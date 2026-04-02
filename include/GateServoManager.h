#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "Config.h"

class GateServoManager {
public:
    GateServoManager();

    void begin();
    void openGate();
    void closeGate();

private:
    Servo gateServo;
    int currentAngle;

    void moveSmooth(int targetAngle);
};