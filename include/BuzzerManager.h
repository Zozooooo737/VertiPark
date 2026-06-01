#pragma once
#include <Arduino.h>
#include "config.h"

// =============================================================================
//                  BUZZER MANAGER (non-bloquant)
// =============================================================================
// Pilote le buzzer passif via tone() / noTone() sans aucun delay().
//
// Patterns disponibles :
//  - SUCCESS    : 2 bips courts aigus (acces accorde)
//  - ERROR      : 1 bip long grave (erreur / refus)
//  - ALERT      : 3 bips moyens (rotation / danger)
//  - SYNC       : pattern continu (bip toutes les X ms) synchronisable avec LED
//  - CONTINUOUS : son continu (utilise pour "Quittez la zone")
//  - HAPPY      : sequence enjouee (utilise pour "Merci")
//
// Le SystemManager peut interroger isBusy() pour synchroniser visuel/sonore.
// =============================================================================

enum BuzzerPattern {
    BUZZ_NONE,
    BUZZ_SINGLE,
    BUZZ_SUCCESS,
    BUZZ_ERROR,
    BUZZ_ALERT,
    BUZZ_SYNC,
    BUZZ_CONTINUOUS,
    BUZZ_HAPPY
};

class BuzzerManager {
public:
    BuzzerManager();

    void begin();
    void update(uint32_t now);

    // ---- Patterns prets a l'emploi ----
    void playSuccess();
    void playError();
    void playAlert();
    void playHappy();
    void startSync(uint32_t periodMs = T_LED_BLINK_NORMAL_MS);
    void startContinuous();
    void stop();

    // ---- Bip ponctuel personnalise ----
    void playBeep(int frequency, uint32_t durationMs);

    // ---- Etat ----
    bool isBusy() const;
    BuzzerPattern getPattern() const;

    // ---- Synchronisation LED externe ----
    // Renvoie true UNE SEULE FOIS au moment ou un bip "sync" demarre.
    // Utile pour faire clignoter une LED en phase avec le buzzer.
    bool consumeSyncTick();

private:
    BuzzerPattern _pattern;
    uint32_t _stateStart;
    uint32_t _patternStart;
    uint8_t  _step;
    bool _toneActive;
    uint32_t _syncPeriod;
    bool _syncTick;
    int  _currentFreq;
    uint32_t _currentDuration;

    void startTone(int freq, uint32_t durationMs);
    void stopTone();
};
