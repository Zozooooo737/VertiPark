#include "GateServoManager.h"

GateServoManager::GateServoManager() {
    currentAngle = GATE_CLOSED_ANGLE;
}

void GateServoManager::begin() {
    gateServo.attach(GATE_SERVO_PIN);
    gateServo.write(currentAngle);
}

void GateServoManager::openGate() {
    moveSmooth(GATE_OPEN_ANGLE);
}

void GateServoManager::closeGate() {
    moveSmooth(GATE_CLOSED_ANGLE);
}

void GateServoManager::moveSmooth(int targetAngle) {
    if (currentAngle < targetAngle) {
        for (int angle = currentAngle; angle <= targetAngle; angle++) {
            gateServo.write(angle);
            delay(10);
        }
    } else {
        for (int angle = currentAngle; angle >= targetAngle; angle--) {
            gateServo.write(angle);
            delay(10);
        }
    }
    currentAngle = targetAngle;
}