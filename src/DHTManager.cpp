#include "DHTManager.h"

DHTManager::DHTManager() : _dht(DHT_PIN, DHT_TYPE) {}

void DHTManager::begin() {
    _dht.begin();
}

float DHTManager::getTemperature() {
    float t = _dht.readTemperature();
    return isnan(t) ? -999.0 : t; // Retourne une valeur erreur si lecture échoue
}

float DHTManager::getHumidity() {
    float h = _dht.readHumidity();
    return isnan(h) ? -999.0 : h;
}

bool DHTManager::isDataValid() {
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();
    return !isnan(t) && !isnan(h);
}