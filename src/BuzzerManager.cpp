#include "BuzzerManager.h"

// =============================================================================
//             BUZZER MANAGER - IMPLEMENTATION (non-bloquant)
// =============================================================================

BuzzerManager::BuzzerManager()
    : _pattern(BUZZ_NONE),
      _stateStart(0),
      _patternStart(0),
      _step(0),
      _toneActive(false),
      _syncPeriod(T_LED_BLINK_NORMAL_MS),
      _syncTick(false),
      _currentFreq(0),
      _currentDuration(0) {}

// -----------------------------------------------------------------------------
void BuzzerManager::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    stopTone();
    _pattern = BUZZ_NONE;
}

// -----------------------------------------------------------------------------
void BuzzerManager::update(uint32_t now) {

    // ---- Si aucun pattern actif : rien a faire ----
    if (_pattern == BUZZ_NONE) return;

    // ---- Si un tone est actif : verifier sa fin ----
    if (_toneActive) {
        if (now - _stateStart >= _currentDuration) {
            stopTone();
            _stateStart = now;
            _step++;
        } else {
            return;
        }
    } else {
        // Pause inter-bip : verifier sa fin
        if (now - _stateStart < T_BEEP_INTER_MS) return;
    }

    // ---- Avancement du pattern courant ----
    switch (_pattern) {

        // ---------- SINGLE : 1 bip ponctuel ----------
        case BUZZ_SINGLE:
            _pattern = BUZZ_NONE;
            break;

        // ---------- SUCCESS : 2 bips courts aigus ----------
        case BUZZ_SUCCESS:
            if (_step == 1) {
                startTone(BUZZ_FREQ_SUCCESS, T_BEEP_SHORT_MS);
                _stateStart = now;
            } else {
                _pattern = BUZZ_NONE;
            }
            break;

        // ---------- ERROR : 1 bip long grave ----------
        case BUZZ_ERROR:
            _pattern = BUZZ_NONE;
            break;

        // ---------- ALERT : 3 bips moyens ----------
        case BUZZ_ALERT:
            if (_step < 3) {
                startTone(BUZZ_FREQ_ALERT, T_BEEP_SHORT_MS);
                _stateStart = now;
            } else {
                _pattern = BUZZ_NONE;
            }
            break;

        // ---------- HAPPY : pattern "merci" enjoue ----------
        case BUZZ_HAPPY: {
            // Suite ascendante (4 notes)
            static const int freqs[4] = { 1500, 1800, 2100, 2400 };
            if (_step < 4) {
                startTone(freqs[_step], T_BEEP_SHORT_MS);
                _stateStart = now;
            } else {
                _pattern = BUZZ_NONE;
            }
            break;
        }

        // ---------- SYNC : bip periodique pour LED ----------
        case BUZZ_SYNC: {
            // Periode = _syncPeriod, bip = T_BEEP_SHORT_MS
            startTone(BUZZ_FREQ_ALERT, T_BEEP_SHORT_MS);
            _stateStart = now;
            _syncTick = true;  // signal pour LED externe

            // Reajuster la pause inter-bip pour respecter la periode
            // L'astuce : on ecrase T_BEEP_INTER_MS via _currentDuration logique
            // En pratique : on attend (periode - duration) apres ce bip.
            // Donc on retarde le prochain tick :
            _currentDuration = T_BEEP_SHORT_MS;
            break;
        }

        // ---------- CONTINUOUS : son continu ----------
        case BUZZ_CONTINUOUS:
            // Tone deja actif (declenche dans startContinuous).
            // On reste dans cet etat tant que stop() n'est pas appele.
            break;

        default:
            _pattern = BUZZ_NONE;
            break;
    }
}

// =============================================================================
//                          PATTERNS PUBLICS
// =============================================================================

void BuzzerManager::playSuccess() {
    stopTone();
    _pattern = BUZZ_SUCCESS;
    _patternStart = millis();
    _step = 0;
    startTone(BUZZ_FREQ_SUCCESS, T_BEEP_SHORT_MS);
    _stateStart = _patternStart;
}

void BuzzerManager::playError() {
    stopTone();
    _pattern = BUZZ_ERROR;
    _patternStart = millis();
    _step = 0;
    startTone(BUZZ_FREQ_ERROR, T_BEEP_LONG_MS);
    _stateStart = _patternStart;
}

void BuzzerManager::playAlert() {
    stopTone();
    _pattern = BUZZ_ALERT;
    _patternStart = millis();
    _step = 0;
    startTone(BUZZ_FREQ_ALERT, T_BEEP_SHORT_MS);
    _stateStart = _patternStart;
}

void BuzzerManager::playHappy() {
    stopTone();
    _pattern = BUZZ_HAPPY;
    _patternStart = millis();
    _step = 0;
    startTone(1500, T_BEEP_SHORT_MS);
    _stateStart = _patternStart;
}

void BuzzerManager::startSync(uint32_t periodMs) {
    stopTone();
    _pattern = BUZZ_SYNC;
    _patternStart = millis();
    _step = 0;
    _syncPeriod = periodMs;
    _syncTick = true;  // premier tick immediat
    startTone(BUZZ_FREQ_ALERT, T_BEEP_SHORT_MS);
    _stateStart = _patternStart;
}

void BuzzerManager::startContinuous() {
    stopTone();
    _pattern = BUZZ_CONTINUOUS;
    _patternStart = millis();
    _step = 0;
    // Tone "infini" : on lance sans duree limite
    tone(BUZZER_PIN, BUZZ_FREQ_INFO);
    _toneActive = true;
    _currentFreq = BUZZ_FREQ_INFO;
    _currentDuration = 0xFFFFFFFF;  // tres grand pour pas que update le coupe
    _stateStart = _patternStart;
}

void BuzzerManager::stop() {
    stopTone();
    _pattern = BUZZ_NONE;
    _step = 0;
    _syncTick = false;
}

// -----------------------------------------------------------------------------
void BuzzerManager::playBeep(int frequency, uint32_t durationMs) {
    stopTone();
    _pattern = BUZZ_SINGLE;
    _patternStart = millis();
    _step = 0;
    startTone(frequency, durationMs);
    _stateStart = _patternStart;
}

// =============================================================================
//                          ETAT / SYNC LED
// =============================================================================

bool BuzzerManager::isBusy() const {
    return _pattern != BUZZ_NONE;
}

BuzzerPattern BuzzerManager::getPattern() const {
    return _pattern;
}

bool BuzzerManager::consumeSyncTick() {
    if (_syncTick) {
        _syncTick = false;
        return true;
    }
    return false;
}

// =============================================================================
//                          HELPERS INTERNES
// =============================================================================

void BuzzerManager::startTone(int freq, uint32_t durationMs) {
    tone(BUZZER_PIN, freq);
    _currentFreq = freq;
    _currentDuration = durationMs;
    _toneActive = true;
}

void BuzzerManager::stopTone() {
    if (_toneActive) {
        noTone(BUZZER_PIN);
        _toneActive = false;
    }
}
