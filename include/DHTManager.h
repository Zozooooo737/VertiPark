#pragma once
#include <Arduino.h>
#include <DHT.h>
#include "config.h"

// =============================================================================
//                       DHT MANAGER (non-bloquant)
// =============================================================================
// Le capteur DHT22 est lent (lecture ~250 ms). Pour eviter de bloquer la FSM :
//
//  - update(now) effectue une lecture une seule fois toutes les T_DHT_REFRESH_MS
//  - Les getters retournent la derniere valeur valide en cache
//  - Une lecture echouee NE remplace PAS la derniere valeur valide
//  - isDataValid() indique si au moins une lecture a reussi depuis begin()
//
// La lecture DHT en soi reste bloquante (limite librairie), mais elle est
// peu frequente (toutes les 5s) et tres courte (~25ms en mode bus optimise).
// =============================================================================

class DHTManager {
public:
    DHTManager();

    void begin();
    void update(uint32_t now);

    // Forcer une lecture immediate (a utiliser uniquement hors FSM critique)
    void forceRefresh();

    // ---- Acces aux donnees (cache) ----
    float getTemperature() const;     // -999.0 si pas encore de donnees
    float getHumidity()    const;     // -999.0 si pas encore de donnees
    bool  isDataValid()    const;

    uint32_t getLastUpdate() const;

private:
    DHT _dht;

    float _cachedTemp;
    float _cachedHumidity;
    bool  _hasValidData;

    uint32_t _lastRead;
    uint32_t _lastValid;
};
