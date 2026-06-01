#include "SensorManager.h"

// =============================================================================
//             SENSOR MANAGER - IMPLEMENTATION (non-bloquant)
// =============================================================================

SensorManager::SensorManager()
    : _rawIrEntry(0), _rawIrExit(0), _rawHall(0), _rawAir(0),
      _stableIrEntry(false), _stableIrExit(false), _stableHall(false),
      _candidateIrEntry(false), _candidateIrExit(false), _candidateHall(false),
      _candidateIrEntryTime(0), _candidateIrExitTime(0), _candidateHallTime(0),
      _evtIrEntryRising(false), _evtIrEntryFalling(false), _evtHallRising(false),
      _lastPoll(0) {}

// -----------------------------------------------------------------------------
void SensorManager::begin() {
    pinMode(MUX_SEL0_PIN, OUTPUT);
    pinMode(MUX_SEL1_PIN, OUTPUT);
    pinMode(MUX_SIG_PIN, INPUT);

    // Premiere lecture pour amorcer le cache
    _rawIrEntry = readMuxChannel(MUX_IR_ENTRY_CHANNEL);
    _rawIrExit  = readMuxChannel(MUX_IR_EXIT_CHANNEL);
    _rawHall    = readMuxChannel(MUX_HALL_CHANNEL);
    _rawAir     = readMuxChannel(MUX_AIR_QUALITY_CHANNEL);

#if LOG_LEVEL >= 1
    Serial.println(F("[SENSOR] Initialise."));
#endif
}

// -----------------------------------------------------------------------------
void SensorManager::update(uint32_t now) {
    if (now - _lastPoll < T_SENSOR_POLL_MS) return;
    _lastPoll = now;

    // ---- Lecture des 4 canaux mux ----
    _rawIrEntry = readMuxChannel(MUX_IR_ENTRY_CHANNEL);
    _rawIrExit  = readMuxChannel(MUX_IR_EXIT_CHANNEL);
    _rawHall    = readMuxChannel(MUX_HALL_CHANNEL);
    _rawAir     = readMuxChannel(MUX_AIR_QUALITY_CHANNEL);

    // ---- Detection candidat IR Entree ----
    bool newIrEntry = (_rawIrEntry > IR_THRESHOLD);
    if (newIrEntry != _candidateIrEntry) {
        _candidateIrEntry = newIrEntry;
        _candidateIrEntryTime = now;
    } else if (newIrEntry != _stableIrEntry &&
               (now - _candidateIrEntryTime) >= IR_DEBOUNCE_MS) {
        // Stabilise -> generer evenement front
        if (newIrEntry) _evtIrEntryRising  = true;
        else            _evtIrEntryFalling = true;
        _stableIrEntry = newIrEntry;
    }

    // ---- IR Sortie (defectueux mais lecture conservee) ----
    bool newIrExit = (_rawIrExit > IR_THRESHOLD);
    if (newIrExit != _candidateIrExit) {
        _candidateIrExit = newIrExit;
        _candidateIrExitTime = now;
    } else if (newIrExit != _stableIrExit &&
               (now - _candidateIrExitTime) >= IR_DEBOUNCE_MS) {
        _stableIrExit = newIrExit;
    }

    // ---- Hall (capteur digital actif LOW une fois converti) ----
    // Sur entree analogique : valeur faible = aimant detecte
    bool newHall = (_rawHall < (IR_THRESHOLD / 2));
    if (newHall != _candidateHall) {
        _candidateHall = newHall;
        _candidateHallTime = now;
    } else if (newHall != _stableHall &&
               (now - _candidateHallTime) >= IR_DEBOUNCE_MS) {
        if (newHall) _evtHallRising = true;
        _stableHall = newHall;
    }
}

// =============================================================================
//                            GETTERS
// =============================================================================

bool SensorManager::isCarAtEntry() const   { return _stableIrEntry; }
bool SensorManager::isCarAtExit()  const   { return _stableIrExit;  }
bool SensorManager::isHallTriggered() const{ return _stableHall;    }

int   SensorManager::getIrEntryRaw() const { return _rawIrEntry; }
int   SensorManager::getIrExitRaw()  const { return _rawIrExit;  }
int   SensorManager::getHallRaw()    const { return _rawHall;    }
float SensorManager::getAirQuality() const { return (float)_rawAir; }

// =============================================================================
//                            EVENEMENTS (consommables)
// =============================================================================

bool SensorManager::consumeIrEntryRising() {
    if (_evtIrEntryRising) { _evtIrEntryRising = false; return true; }
    return false;
}

bool SensorManager::consumeIrEntryFalling() {
    if (_evtIrEntryFalling) { _evtIrEntryFalling = false; return true; }
    return false;
}

bool SensorManager::consumeHallRising() {
    if (_evtHallRising) { _evtHallRising = false; return true; }
    return false;
}

// =============================================================================
//                            LECTURE MUX
// =============================================================================

int SensorManager::readMuxChannel(uint8_t channel) {
    digitalWrite(MUX_SEL0_PIN, channel & 0x01);
    digitalWrite(MUX_SEL1_PIN, (channel >> 1) & 0x01);
    delayMicroseconds(5);  // settle mux (5us, negligeable et non-bloquant en pratique)
    return analogRead(MUX_SIG_PIN);
}
