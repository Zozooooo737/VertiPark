#include "LedsManager.h"

LedsManager::LedsManager() {
    // Par défaut au démarrage, on considère toutes les places comme libres
    for (int i = 0; i < NUM_PLACES; i++) {
        _placesStatus[i] = false; 
    }
}

void LedsManager::begin() {
    pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
    pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
    pinMode(SHIFT_REG_CLOCK_PIN, OUTPUT);

    // --- INITIALISATION DES NOUVELLES LEDS UTILISATEUR ---
    pinMode(PIN_LED_BLUE, OUTPUT);
    pinMode(PIN_LED_ORANGE, OUTPUT);
    
    // On les éteint par défaut au démarrage
    digitalWrite(PIN_LED_BLUE, LOW);
    digitalWrite(PIN_LED_ORANGE, LOW);
    
    // Premier affichage (Toutes les places libres au démarrage)
    updateShiftRegister();
}

void LedsManager::setPlaceStatus(int placeIndex, bool isOccupied) {
    if (placeIndex >= 0 && placeIndex < NUM_PLACES) {
        _placesStatus[placeIndex] = isOccupied;
        updateShiftRegister();
    }
}

void LedsManager::setAllPlacesFree() {
    for (int i = 0; i < NUM_PLACES; i++) {
        _placesStatus[i] = false;
    }
    updateShiftRegister();
}

void LedsManager::setAllPlacesOccupied() {
    for (int i = 0; i < NUM_PLACES; i++) {
        _placesStatus[i] = true;
    }
    updateShiftRegister();
}

bool LedsManager::isPlaceOccupied(int placeIndex) const {
    if (placeIndex >= 0 && placeIndex < NUM_PLACES) {
        return _placesStatus[placeIndex];
    }
    return false;
}

void LedsManager::updateShiftRegister() {
    byte maskData = 0;

    // Préparation de l'octet pour QB à QH (décalage de i + 1)
    for (int i = 0; i < NUM_PLACES; i++) {
        // Si la place est LIBRE (!isOccupied), on veut un 1 (allume le Vert, éteint le Rouge via NOT)
        // Si elle est OCCUPÉE, on laisse le bit à 0 (éteint le Vert, allume le Rouge via NOT)
        if (!_placesStatus[i]) {
            maskData |= (1 << (i + 1)); // Décale de i+1 pour ignorer QA et cibler QB à QH
        }
    }

    // --- REPRISE DE CONTRÔLE LOGIQUE ---
    // On force l'ESP32 à repasser ces pins en sorties standards (anti-conflit I2C/OLED)
    pinMode(SHIFT_REG_DATA_PIN, OUTPUT);
    pinMode(SHIFT_REG_LATCH_PIN, OUTPUT);
    pinMode(SHIFT_REG_CLOCK_PIN, OUTPUT);

    // Envoi des données avec la fonction shiftOut qui a fait ses preuves
    digitalWrite(SHIFT_REG_LATCH_PIN, LOW);
    
    // Utilisation de MSBFIRST comme dans ton test réussi
    shiftOut(SHIFT_REG_DATA_PIN, SHIFT_REG_CLOCK_PIN, MSBFIRST, maskData);
    
    digitalWrite(SHIFT_REG_LATCH_PIN, HIGH);
}

void LedsManager::setBlueLed(bool turnOn) {
    pinMode(PIN_LED_BLUE, OUTPUT); // Sécurité anti-conflit
    digitalWrite(PIN_LED_BLUE, turnOn ? HIGH : LOW);
}

// --- PILOTAGE DE LA LED ORANGE ---
void LedsManager::setOrangeLed(bool turnOn) {
    pinMode(PIN_LED_ORANGE, OUTPUT); // Sécurité anti-conflit
    digitalWrite(PIN_LED_ORANGE, turnOn ? HIGH : LOW);
}