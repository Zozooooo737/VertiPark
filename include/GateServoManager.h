#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

// =============================================================================
//                    GATE SERVO MANAGER (Barriere)
// =============================================================================
// Architecture non-bloquante :
//  - openGate()  / closeGate() declenchent le mouvement
//  - update(now) avance d'un step toutes les GATE_STEP_INTERVAL_MS
//  - getProgress() retourne 0..100 (utile pour OLED progress bar)
//  - isMoving() / isOpen() / isClosed() pour la FSM superieure
//
// Le servo est attache uniquement pendant le mouvement (economie + silence).
// =============================================================================

enum GateState {
    GATE_S_CLOSED,
    GATE_S_OPENING,
    GATE_S_OPEN,
    GATE_S_CLOSING
};

class GateServoManager {
public:
    GateServoManager();

    void begin();
    void update(uint32_t now);

    // ---- Commandes ----
    void openGate();
    void closeGate();

    // ---- Etat ----
    bool isMoving() const;
    bool isOpen() const;
    bool isClosed() const;
    GateState getState() const;

    // Progression 0..100 (pendant OPENING ou CLOSING)
    int getProgress() const;

    // Angle courant (debug)
    int getCurrentAngle() const;

private:
    Servo _servo;
    bool  _attached;
    GateState _state;

    int _currentAngle;
    int _targetAngle;
    uint32_t _lastStep;

    void attachServo();
    void detachServo();
    void startMove(int target, GateState movingState);
};
