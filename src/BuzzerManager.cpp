#include "BuzzerManager.h"

BuzzerManager::BuzzerManager() {}

void BuzzerManager::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    stop();
}

void BuzzerManager::beep(int frequency, int duration) {
    tone(BUZZER_PIN, frequency, duration); 
    // tone() gère lui-même le démarrage, la durée et l'arrêt
    delay(duration + 10); 
}

void BuzzerManager::playSuccess() {
    beep(2000, 100); // Bip court et aigu
}

void BuzzerManager::playError() {
    beep(500, 300);  // Bip long et grave
}

void BuzzerManager::playAlert() {
    for(int i = 0; i < 3; i++) {
        beep(1500, 100);
        delay(50);
    }
}

void BuzzerManager::stop() {
    noTone(BUZZER_PIN);
}