#include "PivotServoManager.h"

PivotServoManager::PivotServoManager() {
    currentSpeed = PIVOT_STOP;
}

void PivotServoManager::begin() {
    pivotServo.attach(PIVOT_SERVO_PIN);
    pivotServo.write(PIVOT_STOP);
}

void PivotServoManager::rotateCW() {
    pivotServo.write(PIVOT_CW);
    currentSpeed = PIVOT_CW;
}

void PivotServoManager::rotateCCW() {
    pivotServo.write(PIVOT_CCW);
    currentSpeed = PIVOT_CCW;
}

void PivotServoManager::stop() {
    pivotServo.write(PIVOT_STOP);
    currentSpeed = PIVOT_STOP;
}