#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class GateServoManager {
private:
    Servo gateServo;
    int currentAngle;

    void moveSmooth(int targetAngle);

public:
    GateServoManager();

    void begin();
    void openGate();
    void closeGate();

};