#include "GateServoManager.h"

GateServoManager::GateServoManager() {
    currentAngle = GATE_CLOSED_ANGLE;
}

void GateServoManager::begin() {
    gateServo.attach(GATE_SERVO_PIN);
    gateServo.write(currentAngle);
    gateServo.detach();
}

void GateServoManager::openGate() {
    gateServo.attach(GATE_SERVO_PIN);
    moveSmooth(GATE_OPEN_ANGLE);
    gateServo.detach();
}

void GateServoManager::closeGate() {
    gateServo.attach(GATE_SERVO_PIN); 
    moveSmooth(GATE_CLOSED_ANGLE);
    gateServo.detach();
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