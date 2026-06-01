#include "PivotServoManager.h"

// =============================================================================
//                PIVOT SERVO MANAGER - IMPLEMENTATION
// =============================================================================
// Voir PivotServoManager.h pour la specification complete.
// =============================================================================

// -----------------------------------------------------------------------------
// Constructeur
// -----------------------------------------------------------------------------
PivotServoManager::PivotServoManager()
    : _attached(false),
      _state(PIVOT_S_UNINIT),
      _calibrated(false),
      _lastFb(0),
      _turnCount(0),
      _currentAngleRaw(0.0f),
      _pivotOffset(0.0f),
      _targetAngleRaw(0.0f),
      _targetPlace(-1),
      _targetReached(false),
      _moveStartAbs(0.0f),
      _moveTargetAbs(0.0f),
      _currentAbs(0.0f),
      _lastFbPoll(0),
      _moveStartTime(0),
      _stopRequestTime(0) {}

// -----------------------------------------------------------------------------
// begin()
// -----------------------------------------------------------------------------
void PivotServoManager::begin() {
    pinMode(PIVOT_FB_PIN, INPUT);

    // Premiere lecture du FB pour initialiser le suivi
    _lastFb = readFbRaw();
    _currentAngleRaw = fbToDeg(_lastFb);
    _currentAbs = _currentAngleRaw;
    _turnCount = 0;

    _state = PIVOT_S_IDLE;
    _calibrated = false;

#if LOG_LEVEL >= 1
    Serial.print(F("[PIVOT] Init - FB brut="));
    Serial.print(_lastFb);
    Serial.print(F(" angle="));
    Serial.println(_currentAngleRaw);
#endif
}

// -----------------------------------------------------------------------------
// update()
// -----------------------------------------------------------------------------
void PivotServoManager::update(uint32_t now) {
    // ---- Lecture periodique du FB pour suivre position ----
    if (now - _lastFbPoll >= PIVOT_FB_POLL_MS) {
        _lastFbPoll = now;
        updateFeedback();
    }

    // ---- FSM ----
    switch (_state) {

        case PIVOT_S_UNINIT:
        case PIVOT_S_IDLE:
        case PIVOT_S_ERROR:
            // Rien a faire
            break;

        // -----------------------------------------------------------------
        case PIVOT_S_CALIBRATING: {
            // Securite : timeout de calibration
            if (now - _moveStartTime >= T_CALIBRATION_TIMEOUT_MS) {
                writeStop();
                _state = PIVOT_S_ERROR;
#if LOG_LEVEL >= 1
                Serial.println(F("[PIVOT] ERREUR : timeout calibration."));
#endif
            }
            // L'evenement Hall declenche notifyHallTrigger() depuis SystemManager
            break;
        }

        // -----------------------------------------------------------------
        case PIVOT_S_MOVING: {
            // Securite : timeout global
            if (now - _moveStartTime >= PIVOT_ROTATION_TIMEOUT) {
                writeStop();
                _state = PIVOT_S_ERROR;
#if LOG_LEVEL >= 1
                Serial.println(F("[PIVOT] ERREUR : timeout rotation."));
#endif
                break;
            }

            // Securite anti-tour-complet :
            // Si on a parcouru plus de 360 deg sans atteindre la cible -> ABORT.
            float traveled = fabsf(_currentAbs - _moveStartAbs);
            float requested = fabsf(_moveTargetAbs - _moveStartAbs);
            if (traveled > requested + 30.0f && traveled > 360.0f - 30.0f) {
                writeStop();
                _state = PIVOT_S_ERROR;
#if LOG_LEVEL >= 1
                Serial.println(F("[PIVOT] ERREUR : derive >360 deg detectee."));
#endif
                break;
            }

            // Verification arrivee (en degres absolus)
            float remaining = _moveTargetAbs - _currentAbs;
            if (fabsf(remaining) <= (PIVOT_POS_TOLERANCE * 360.0f / 4096.0f)) {
                writeStop();
                _stopRequestTime = now;
                _state = PIVOT_S_STOPPING;
                _targetReached = true;
#if LOG_LEVEL >= 1
                Serial.print(F("[PIVOT] Cible atteinte. Angle="));
                Serial.println(_currentAngleRaw);
#endif
            }
            break;
        }

        // -----------------------------------------------------------------
        case PIVOT_S_STOPPING: {
            // Attente avant detach pour permettre arret mecanique propre
            if (now - _stopRequestTime >= PIVOT_DETACH_DELAY_MS) {
                detachServo();
                _state = PIVOT_S_IDLE;
            }
            break;
        }
    }
}

// =============================================================================
//                       CALIBRATION
// =============================================================================

void PivotServoManager::startCalibration() {
    if (_state == PIVOT_S_CALIBRATING) return;

    attachServo();
    writeSpeed(PIVOT_CW_SLOW);
    _state = PIVOT_S_CALIBRATING;
    _moveStartTime = millis();

#if LOG_LEVEL >= 1
    Serial.println(F("[PIVOT] Calibration : recherche Hall..."));
#endif
}

void PivotServoManager::notifyHallTrigger() {
    // Hall = place 3 alignee (reference centrale)
    // L'angle brut FB courant correspond physiquement a PIVOT_POS_3_DEG.
    // => offset = angle_logique_voulu - angle_raw_actuel
    _pivotOffset = PIVOT_POS_3_DEG - _currentAngleRaw;

    // Normaliser dans 0..360
    while (_pivotOffset < 0.0f)    _pivotOffset += 360.0f;
    while (_pivotOffset >= 360.0f) _pivotOffset -= 360.0f;

    _calibrated = true;

    // Si on etait en calibration, on arrete
    if (_state == PIVOT_S_CALIBRATING) {
        writeStop();
        _stopRequestTime = millis();
        _state = PIVOT_S_STOPPING;
    }

#if LOG_LEVEL >= 1
    Serial.print(F("[PIVOT] Calibration OK. Offset="));
    Serial.println(_pivotOffset);
#endif
}

bool PivotServoManager::isCalibrated() const {
    return _calibrated;
}

// =============================================================================
//                       COMMANDES DE DEPLACEMENT
// =============================================================================

bool PivotServoManager::goToPlace(uint8_t placeIndex) {
    if (placeIndex >= NUM_PLACES) return false;
    if (!_calibrated)             return false;
    if (isBusy())                 return false;

    _targetPlace = placeIndex;
    float targetLogical = placeToAngle(placeIndex);
    return goToAngle(targetLogical);
}

bool PivotServoManager::goToAngle(float targetDeg) {
    if (!_calibrated)              return false;
    if (_state != PIVOT_S_IDLE)    return false;

    // Conversion angle logique -> angle brut FB
    float targetRaw = wrap360(targetDeg - _pivotOffset);
    _targetAngleRaw = targetRaw;

    // Calcul delta court (entre -180 et +180)
    float delta = shortestDelta(_currentAngleRaw, targetRaw);

    // Si deja a destination (tolerance ~5 deg)
    if (fabsf(delta) < (PIVOT_POS_TOLERANCE * 360.0f / 4096.0f)) {
        _targetReached = true;
#if LOG_LEVEL >= 1
        Serial.println(F("[PIVOT] Deja sur la cible, pas de rotation."));
#endif
        return true;
    }

    // REGLE CRITIQUE : on n'execute JAMAIS plus de 360 deg.
    // shortestDelta() renvoie deja un delta dans (-180..+180], donc OK.
    // Securite supplementaire :
    if (fabsf(delta) >= 360.0f - 5.0f) {
#if LOG_LEVEL >= 1
        Serial.println(F("[PIVOT] REFUS : delta >= 360 deg."));
#endif
        _state = PIVOT_S_ERROR;
        return false;
    }

    // Position absolue de depart et d'arrivee
    _moveStartAbs = _currentAbs;
    _moveTargetAbs = _currentAbs + delta;
    _targetReached = false;

    // Choix du sens
    attachServo();
    if (delta > 0) {
        writeSpeed(PIVOT_CW);
    } else {
        writeSpeed(PIVOT_CCW);
    }
    _state = PIVOT_S_MOVING;
    _moveStartTime = millis();

#if LOG_LEVEL >= 1
    Serial.print(F("[PIVOT] GoTo angle="));
    Serial.print(targetDeg);
    Serial.print(F(" delta="));
    Serial.print(delta);
    Serial.println(F(" deg"));
#endif
    return true;
}

void PivotServoManager::stopNow() {
    if (_state == PIVOT_S_MOVING || _state == PIVOT_S_CALIBRATING) {
        writeStop();
        _stopRequestTime = millis();
        _state = PIVOT_S_STOPPING;
#if LOG_LEVEL >= 1
        Serial.println(F("[PIVOT] Stop demande."));
#endif
    }
}

void PivotServoManager::resetError() {
    if (_state == PIVOT_S_ERROR) {
        detachServo();
        _state = PIVOT_S_IDLE;
    }
}

// =============================================================================
//                       ETAT / LECTURES
// =============================================================================

bool PivotServoManager::isBusy() const {
    return (_state == PIVOT_S_MOVING) ||
           (_state == PIVOT_S_CALIBRATING) ||
           (_state == PIVOT_S_STOPPING);
}

bool PivotServoManager::isAtTarget() const {
    return _targetReached && (_state == PIVOT_S_IDLE);
}

bool PivotServoManager::hasError() const {
    return _state == PIVOT_S_ERROR;
}

PivotState PivotServoManager::getState() const {
    return _state;
}

float PivotServoManager::getCurrentAngle() const {
    // Angle logique = angle brut + offset (modulo 360)
    return wrap360(_currentAngleRaw + _pivotOffset);
}

int PivotServoManager::getCurrentPlace() const {
    if (!_calibrated) return -1;

    float current = getCurrentAngle();
    float bestDelta = 1e9;
    int   bestPlace = 0;

    for (int i = 0; i < NUM_PLACES; i++) {
        float d = fabsf(shortestDelta(current, placeToAngle(i)));
        if (d < bestDelta) {
            bestDelta = d;
            bestPlace = i;
        }
    }
    return bestPlace;
}

int PivotServoManager::getTargetPlace() const {
    return _targetPlace;
}

float PivotServoManager::getRotationProgress() const {
    float total = _moveTargetAbs - _moveStartAbs;
    if (fabsf(total) < 0.001f) return 1.0f;
    float done = _currentAbs - _moveStartAbs;
    float p = done / total;
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    return p;
}

// =============================================================================
//                       HELPERS INTERNES
// =============================================================================

void PivotServoManager::attachServo() {
    if (!_attached) {
        _servo.attach(PIVOT_SERVO_PIN);
        _attached = true;
    }
}

void PivotServoManager::detachServo() {
    if (_attached) {
        _servo.detach();
        _attached = false;
    }
}

int PivotServoManager::readFbRaw() {
    return analogRead(PIVOT_FB_PIN);
}

void PivotServoManager::writeSpeed(int speed) {
    if (!_attached) attachServo();
    _servo.write(speed);
}

void PivotServoManager::writeStop() {
    if (_attached) _servo.write(PIVOT_STOP);
}

// -----------------------------------------------------------------------------
// updateFeedback() : lecture FB + detection wrap + mise a jour _currentAbs
// -----------------------------------------------------------------------------
void PivotServoManager::updateFeedback() {
    int fb = readFbRaw();

    // Detection saut 4095 -> 0 (rotation CW positive) ou 0 -> 4095 (CCW negative)
    if (_lastFb > PIVOT_FB_WRAP_HIGH && fb < PIVOT_FB_WRAP_LOW) {
        _turnCount++;
    }
    else if (_lastFb < PIVOT_FB_WRAP_LOW && fb > PIVOT_FB_WRAP_HIGH) {
        _turnCount--;
    }

    _lastFb = fb;
    _currentAngleRaw = fbToDeg(fb);
    _currentAbs = (float)_turnCount * 360.0f + _currentAngleRaw;
}

// -----------------------------------------------------------------------------
// placeToAngle() : index 0..6 -> angle logique en degres
// -----------------------------------------------------------------------------
float PivotServoManager::placeToAngle(uint8_t placeIndex) const {
    switch (placeIndex) {
        case 0: return PIVOT_POS_0_DEG;
        case 1: return PIVOT_POS_1_DEG;
        case 2: return PIVOT_POS_2_DEG;
        case 3: return PIVOT_POS_3_DEG;
        case 4: return PIVOT_POS_4_DEG;
        case 5: return PIVOT_POS_5_DEG;
        case 6: return PIVOT_POS_6_DEG;
        default: return PIVOT_POS_3_DEG;
    }
}

// -----------------------------------------------------------------------------
// wrap360() : normalise un angle dans [0..360)
// -----------------------------------------------------------------------------
float PivotServoManager::wrap360(float a) {
    while (a < 0.0f)    a += 360.0f;
    while (a >= 360.0f) a -= 360.0f;
    return a;
}

// -----------------------------------------------------------------------------
// shortestDelta() : delta signe le plus court dans (-180..+180]
// Positif = sens CW, Negatif = sens CCW
// -----------------------------------------------------------------------------
float PivotServoManager::shortestDelta(float from, float to) {
    float d = wrap360(to - from);
    if (d > 180.0f) d -= 360.0f;
    return d;
}

// -----------------------------------------------------------------------------
// Conversions FB <-> degres
// -----------------------------------------------------------------------------
float PivotServoManager::fbToDeg(int fb) {
    if (fb < 0)    fb = 0;
    if (fb > 4095) fb = 4095;
    return (float)fb * 360.0f / 4096.0f;
}

int PivotServoManager::degToFb(float deg) {
    deg = wrap360(deg);
    int fb = (int)(deg * 4096.0f / 360.0f);
    if (fb < 0)    fb = 0;
    if (fb > 4095) fb = 4095;
    return fb;
}
