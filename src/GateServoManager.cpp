#include "GateServoManager.h"

// =============================================================================
//             GATE SERVO MANAGER - IMPLEMENTATION (non-bloquant)
// =============================================================================

GateServoManager::GateServoManager()
    : _attached(false),
      _state(GATE_S_CLOSED),
      _currentAngle(GATE_CLOSED_ANGLE),
      _targetAngle(GATE_CLOSED_ANGLE),
      _lastStep(0) {}

// -----------------------------------------------------------------------------
void GateServoManager::begin() {
    attachServo();
    _servo.write(GATE_CLOSED_ANGLE);
    _currentAngle = GATE_CLOSED_ANGLE;
    _targetAngle  = GATE_CLOSED_ANGLE;
    _state        = GATE_S_CLOSED;
    _lastStep     = millis();

#if LOG_LEVEL >= 1
    Serial.println(F("[GATE] Init - barriere fermee."));
#endif
}

// -----------------------------------------------------------------------------
void GateServoManager::update(uint32_t now) {
    // ---- Detach automatique en repos (apres 500ms d'immobilite) ----
    if ((_state == GATE_S_CLOSED || _state == GATE_S_OPEN) && _attached) {
        if (now - _lastStep >= 500) {
            detachServo();
        }
        return;
    }

    // ---- Avancement progressif ----
    if (_state == GATE_S_OPENING || _state == GATE_S_CLOSING) {
        if (now - _lastStep < GATE_STEP_INTERVAL_MS) return;
        _lastStep = now;

        if (!_attached) attachServo();

        if (_currentAngle < _targetAngle) {
            _currentAngle += GATE_STEP_DEG;
            if (_currentAngle > _targetAngle) _currentAngle = _targetAngle;
        } else if (_currentAngle > _targetAngle) {
            _currentAngle -= GATE_STEP_DEG;
            if (_currentAngle < _targetAngle) _currentAngle = _targetAngle;
        }

        _servo.write(_currentAngle);

        // ---- Arrivee a destination ----
        if (_currentAngle == _targetAngle) {
            if (_state == GATE_S_OPENING) {
                _state = GATE_S_OPEN;
#if LOG_LEVEL >= 1
                Serial.println(F("[GATE] Open"));
#endif
            } else {
                _state = GATE_S_CLOSED;
#if LOG_LEVEL >= 1
                Serial.println(F("[GATE] Closed"));
#endif
            }
            _lastStep = now;
        }
    }
}

// -----------------------------------------------------------------------------
void GateServoManager::openGate() {
    if (_state == GATE_S_OPEN || _state == GATE_S_OPENING) return;
#if LOG_LEVEL >= 1
    Serial.println(F("[GATE] Opening"));
#endif
    startMove(GATE_OPEN_ANGLE, GATE_S_OPENING);
}

void GateServoManager::closeGate() {
    if (_state == GATE_S_CLOSED || _state == GATE_S_CLOSING) return;
#if LOG_LEVEL >= 1
    Serial.println(F("[GATE] Closing"));
#endif
    startMove(GATE_CLOSED_ANGLE, GATE_S_CLOSING);
}

// -----------------------------------------------------------------------------
// startMove : attache le servo et ECRIT immediatement la position courante
// pour eviter un saut parasite vers la position par defaut de la librairie.
// -----------------------------------------------------------------------------
void GateServoManager::startMove(int target, GateState movingState) {
    _targetAngle = target;
    _state       = movingState;
    attachServo();

    // CRITIQUE : ecrire la position courante immediatement pour fixer le PWM
    _servo.write(_currentAngle);

    // Le premier step se fera apres GATE_STEP_INTERVAL_MS
    _lastStep = millis();
}

// -----------------------------------------------------------------------------
bool GateServoManager::isMoving() const {
    return (_state == GATE_S_OPENING) || (_state == GATE_S_CLOSING);
}

bool GateServoManager::isOpen() const   { return _state == GATE_S_OPEN; }
bool GateServoManager::isClosed() const { return _state == GATE_S_CLOSED; }

GateState GateServoManager::getState() const { return _state; }

int GateServoManager::getProgress() const {
    int range = GATE_OPEN_ANGLE - GATE_CLOSED_ANGLE;
    if (range == 0) return 100;

    if (_state == GATE_S_OPENING) {
        int done = _currentAngle - GATE_CLOSED_ANGLE;
        return (done * 100) / range;
    }
    if (_state == GATE_S_CLOSING) {
        int done = GATE_OPEN_ANGLE - _currentAngle;
        return (done * 100) / range;
    }
    if (_state == GATE_S_OPEN)   return 100;
    return 0;
}

int GateServoManager::getCurrentAngle() const {
    return _currentAngle;
}

// -----------------------------------------------------------------------------
void GateServoManager::attachServo() {
    if (!_attached) {
        _servo.attach(GATE_SERVO_PIN);
        _attached = true;
    }
}

void GateServoManager::detachServo() {
    if (_attached) {
        _servo.detach();
        _attached = false;
    }
}
