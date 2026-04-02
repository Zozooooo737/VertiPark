#include "SystemManager.h"
#include "GateServoManager.h"

SystemManager::SystemManager(SensorManager& sensorMgr,
                             GateServoManager& gateMgr,
                             PivotServoManager& pivotMgr)
    : sensorManager(sensorMgr),
      gateManager(gateMgr),
      pivotManager(pivotMgr) {}

void SystemManager::begin() {
    // Rien pour l'instant
}

void SystemManager::process() {

    Serial.println("--------------------");
    Serial.print("Entrée : "); Serial.println(sensorManager.isCarAtEntry());
    Serial.print("Sortie : "); Serial.println(sensorManager.isCarAtExit());
    Serial.print("Light1 : "); Serial.println(sensorManager.getLight(0));
    Serial.print("Light2 : "); Serial.println(sensorManager.getLight(1));
    Serial.print("Air Quality : "); Serial.println(sensorManager.getAirQuality());
    Serial.print("Hall : "); Serial.println(sensorManager.isHallTriggered());
    Serial.println("--------------------");

    if (sensorManager.isCarAtEntry()) {
        Serial.println("Voiture détectée → ouverture barrière");
        gateManager.openGate();
        delay(1000);
        gateManager.closeGate();
    }
}