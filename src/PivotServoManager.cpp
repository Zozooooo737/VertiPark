#include "PivotServoManager.h"

PivotServoManager::PivotServoManager() {
    currentSpeed = PIVOT_STOP;
}

void PivotServoManager::begin() {
    stop(); 
}

void PivotServoManager::rotateCW() {
    if (!pivotServo.attached()) {
        pivotServo.attach(PIVOT_SERVO_PIN);
    }
    pivotServo.write(PIVOT_CW);
    currentSpeed = PIVOT_CW;
}

void PivotServoManager::rotateCCW() {
    if (!pivotServo.attached()) {
        pivotServo.attach(PIVOT_SERVO_PIN);
    }
    pivotServo.write(PIVOT_CCW);
    currentSpeed = PIVOT_CCW;
}

void PivotServoManager::stop() {
    if (!pivotServo.attached()) {
        pivotServo.attach(PIVOT_SERVO_PIN);
    }
    
    pivotServo.write(PIVOT_STOP);
    currentSpeed = PIVOT_STOP;
    
    delay(50); 
    
    pivotServo.detach();
}

void PivotServoManager::detach() {
    if (pivotServo.attached()) {
        pivotServo.detach();
    }
}