#include "SensorManager.h"

SensorManager::SensorManager() {}

void SensorManager::begin() {
    pinMode(MUX_SEL0_PIN, OUTPUT);
    pinMode(MUX_SEL1_PIN, OUTPUT);
    pinMode(MUX_SEL2_PIN, OUTPUT);
    pinMode(MUX_SIG_PIN, INPUT);
}

bool SensorManager::isCarAtEntry() {
    return readMux(MUX_IR_ENTRY_CHANNEL) > IR_THRESHOLD;
}

bool SensorManager::isCarAtExit() {
    return readMux(MUX_IR_EXIT_CHANNEL) > IR_THRESHOLD;
}

int SensorManager::getLight(uint8_t index) {
    if(index == 0) return readMux(MUX_LIGHT1_CHANNEL);
    if(index == 1) return readMux(MUX_LIGHT2_CHANNEL);
    return -1;
}

float SensorManager::getAirQuality() {
    int raw = readMux(MUX_AIR_QUALITY_CHANNEL);
    return raw;
}

bool SensorManager::isHallTriggered() {
    int val = readMux(MUX_HALL_CHANNEL);
    return val == HIGH;
}

int SensorManager::readMux(uint8_t channel) {
    digitalWrite(MUX_SEL0_PIN, channel & 0x01);
    digitalWrite(MUX_SEL1_PIN, (channel >> 1) & 0x01);
    digitalWrite(MUX_SEL2_PIN, (channel >> 2) & 0x01);

    delayMicroseconds(5);
    return analogRead(MUX_SIG_PIN);
}