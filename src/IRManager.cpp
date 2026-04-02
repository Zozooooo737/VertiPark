// #include "IRManager.h"

// IRManager::IRManager() {
//     entryPin = IR_SENSOR_ENTRY_PIN;
//     exitPin = IR_SENSOR_EXIT_PIN;
// }

// void IRManager::begin() {
//     pinMode(entryPin, INPUT);
//     pinMode(exitPin, INPUT);
// }

// bool IRManager::isCarAtEntry() {
//     return analogRead(entryPin) > SEUIL_IR;
// }

// bool IRManager::isCarAtExit() {
//     return analogRead(exitPin) > SEUIL_IR;
// }