#include "DHTManager.h"

// =============================================================================
//               DHT MANAGER - IMPLEMENTATION (cache periodique)
// =============================================================================

DHTManager::DHTManager()
    : _dht(DHT_PIN, DHT_TYPE),
      _cachedTemp(-999.0f),
      _cachedHumidity(-999.0f),
      _hasValidData(false),
      _lastRead(0),
      _lastValid(0) {}

// -----------------------------------------------------------------------------
void DHTManager::begin() {
    _dht.begin();
    _lastRead = 0;
    _lastValid = 0;
    _hasValidData = false;

#if LOG_LEVEL >= 1
    Serial.println(F("[DHT] Initialise."));
#endif
}

// -----------------------------------------------------------------------------
void DHTManager::update(uint32_t now) {
    // Lecture une fois toutes les T_DHT_REFRESH_MS
    if (_lastRead != 0 && (now - _lastRead) < T_DHT_REFRESH_MS) return;

    _lastRead = now;
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        _cachedTemp = t;
        _cachedHumidity = h;
        _hasValidData = true;
        _lastValid = now;

#if LOG_LEVEL >= 2
        Serial.print(F("[DHT] T="));
        Serial.print(t);
        Serial.print(F("C H="));
        Serial.print(h);
        Serial.println(F("%"));
#endif
    } else {
#if LOG_LEVEL >= 1
        Serial.println(F("[DHT] Lecture invalide, cache conserve."));
#endif
    }
}

// -----------------------------------------------------------------------------
void DHTManager::forceRefresh() {
    _lastRead = 0;     // force le prochain update() a relire
}

// =============================================================================
//                            GETTERS
// =============================================================================

float DHTManager::getTemperature() const { return _cachedTemp; }
float DHTManager::getHumidity()    const { return _cachedHumidity; }
bool  DHTManager::isDataValid()    const { return _hasValidData; }
uint32_t DHTManager::getLastUpdate() const { return _lastValid; }
