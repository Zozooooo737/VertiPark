#include "SensorManager.h"

SensorManager::SensorManager() {}

void SensorManager::begin() {
    pinMode(MUX_SEL0_PIN, OUTPUT);
    pinMode(MUX_SEL1_PIN, OUTPUT);
    
    pinMode(MUX_SIG_PIN, INPUT);
}

bool SensorManager::isCarAtEntry() {
    int rawValue = readMuxChannel(MUX_IR_ENTRY_CHANNEL);
    
    // Affichage dans le moniteur série
    Serial.print("IR Entree - Valeur brute : ");
    Serial.println(rawValue);
    
    return rawValue > IR_THRESHOLD;
}

bool SensorManager::isCarAtExit() {
    int rawValue = readMuxChannel(MUX_IR_EXIT_CHANNEL);
    
    // Affichage dans le moniteur série
    Serial.print("IR Sortie - Valeur brute : ");
    Serial.println(rawValue);
    
    return rawValue > IR_THRESHOLD;
}

bool SensorManager::isHallTriggered() {
    int val = readMuxChannel(MUX_HALL_CHANNEL);
    return val == LOW; 
}

float SensorManager::getAirQuality() {
    int raw = readMuxChannel(MUX_AIR_QUALITY_CHANNEL);
    return (float)raw;
}

int SensorManager::readMuxChannel(uint8_t channel) {
    digitalWrite(MUX_SEL0_PIN, channel & 0x01);
    digitalWrite(MUX_SEL1_PIN, (channel >> 1) & 0x01);

    delayMicroseconds(5);
    
    return analogRead(MUX_SIG_PIN);
}