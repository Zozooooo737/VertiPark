#include "LedsManager.h"

// =============================================================================
//                LEDS MANAGER - IMPLEMENTATION (strictement visuel)
// =============================================================================

LedsManager::LedsManager()
    : _blueMode(LED_OFF),
      _orangeMode(LED_OFF),
      _blueOn(false),
      _orangeOn(false),
      _blueLastToggle(0),
      _orangeLastToggle(0),
      _reservedBlinkOn(false),
      _reservedLastToggle(0) {
    for (int i = 0; i < NUM_PLACES; i++) {
        _places[i] = PLACE_FREE;
    }
}

// -----------------------------------------------------------------------------
void LedsManager::begin() {
    pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
    pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
    pinMode(SHIFT_REG_CLOCK_PIN, OUTPUT);

    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_ORANGE, OUTPUT);

    digitalWrite(PIN_LED_BLUE, LOW);
    digitalWrite(PIN_LED_ORANGE, LOW);

    updateShiftRegister();
}

// -----------------------------------------------------------------------------
void LedsManager::update(uint32_t now) {

    // ---- Clignotement places reservees ----
    bool hasReserved = false;
    for (int i = 0; i < NUM_PLACES; i++) {
        if (_places[i] == PLACE_RESERVED) { hasReserved = true; break; }
    }
    if (hasReserved) {
        if (now - _reservedLastToggle >= T_LED_BLINK_NORMAL_MS) {
            _reservedLastToggle = now;
            _reservedBlinkOn = !_reservedBlinkOn;
            updateShiftRegister();
        }
    }

    // ---- LED bleue ----
    uint32_t bluePeriod = periodForMode(_blueMode);
    if (bluePeriod > 0) {
        if (now - _blueLastToggle >= bluePeriod) {
            _blueLastToggle = now;
            _blueOn = !_blueOn;
            applyBlueLed();
        }
    }

    // ---- LED orange ----
    uint32_t orangePeriod = periodForMode(_orangeMode);
    if (orangePeriod > 0) {
        if (now - _orangeLastToggle >= orangePeriod) {
            _orangeLastToggle = now;
            _orangeOn = !_orangeOn;
            applyOrangeLed();
        }
    }
}

// =============================================================================
//                          PLACES
// =============================================================================

void LedsManager::setPlaceState(uint8_t placeIndex, PlaceState state) {
    if (placeIndex >= NUM_PLACES) return;
    _places[placeIndex] = state;
    updateShiftRegister();
}

void LedsManager::setAllPlacesFree() {
    for (int i = 0; i < NUM_PLACES; i++) _places[i] = PLACE_FREE;
    updateShiftRegister();
}

void LedsManager::setAllPlacesOccupied() {
    for (int i = 0; i < NUM_PLACES; i++) _places[i] = PLACE_OCCUPIED;
    updateShiftRegister();
}

PlaceState LedsManager::getPlaceState(uint8_t placeIndex) const {
    if (placeIndex >= NUM_PLACES) return PLACE_FREE;
    return _places[placeIndex];
}

// =============================================================================
//                          LED BLEUE / ORANGE
// =============================================================================

void LedsManager::setBlueMode(LedMode mode) {
    if (_blueMode == mode) return;
    _blueMode = mode;
    _blueLastToggle = millis();
    switch (mode) {
        case LED_OFF:           _blueOn = false; break;
        case LED_ON:            _blueOn = true;  break;
        case LED_EXTERN_SYNC:   _blueOn = false; break;
        default:                _blueOn = true;  break;  // demarre allume
    }
    applyBlueLed();
}

LedMode LedsManager::getBlueMode() const { return _blueMode; }

void LedsManager::setOrangeMode(LedMode mode) {
    if (_orangeMode == mode) return;
    _orangeMode = mode;
    _orangeLastToggle = millis();
    switch (mode) {
        case LED_OFF:           _orangeOn = false; break;
        case LED_ON:            _orangeOn = true;  break;
        case LED_EXTERN_SYNC:   _orangeOn = false; break;
        default:                _orangeOn = true;  break;
    }
    applyOrangeLed();
}

LedMode LedsManager::getOrangeMode() const { return _orangeMode; }

// -----------------------------------------------------------------------------
void LedsManager::toggleBlue() {
    if (_blueMode != LED_EXTERN_SYNC) return;
    _blueOn = !_blueOn;
    applyBlueLed();
}

void LedsManager::toggleOrange() {
    if (_orangeMode != LED_EXTERN_SYNC) return;
    _orangeOn = !_orangeOn;
    applyOrangeLed();
}

// -----------------------------------------------------------------------------
void LedsManager::allOff() {
    setBlueMode(LED_OFF);
    setOrangeMode(LED_OFF);
}

// =============================================================================
//                          HELPERS INTERNES
// =============================================================================

void LedsManager::applyBlueLed() {
    pinMode(PIN_LED_BLUE, OUTPUT);
    digitalWrite(PIN_LED_BLUE, _blueOn ? HIGH : LOW);
}

void LedsManager::applyOrangeLed() {
    pinMode(PIN_LED_ORANGE, OUTPUT);
    digitalWrite(PIN_LED_ORANGE, _orangeOn ? HIGH : LOW);
}

uint32_t LedsManager::periodForMode(LedMode mode) const {
    switch (mode) {
        case LED_BLINK_FAST:   return T_LED_BLINK_FAST_MS;
        case LED_BLINK_NORMAL: return T_LED_BLINK_NORMAL_MS;
        case LED_BLINK_SLOW:   return T_LED_BLINK_SLOW_MS;
        default: return 0;  // 0 = pas d'auto-toggle
    }
}

// -----------------------------------------------------------------------------
// updateShiftRegister() : envoie l'etat des 7 places vers le 74HC595
//
// Mapping bit : QA non utilisee, QB..QH = places 0..6
// Pour chaque place :
//    FREE      -> bit 1 (LED verte allumee, rouge eteinte via NOT)
//    OCCUPIED  -> bit 0 (verte eteinte, rouge allumee)
//    RESERVED  -> bit qui clignote selon _reservedBlinkOn
// -----------------------------------------------------------------------------
void LedsManager::updateShiftRegister() {
    byte maskData = 0;

    for (int i = 0; i < NUM_PLACES; i++) {
        bool bit = false;
        switch (_places[i]) {
            case PLACE_FREE:
                bit = true;  // vert
                break;
            case PLACE_OCCUPIED:
                bit = false; // rouge
                break;
            case PLACE_RESERVED:
                bit = _reservedBlinkOn; // alterne vert/rouge
                break;
        }
        if (bit) {
            maskData |= (1 << (i + 1));
        }
    }

    // Reprise de controle des pins (anti-conflit avec SPI partage)
    pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
    pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
    pinMode(SHIFT_REG_CLOCK_PIN, OUTPUT);

    digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
    shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLOCK_PIN, MSBFIRST, maskData);
    digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
}
