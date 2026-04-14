#include "HardwareTest.h"

HardwareTest::HardwareTest(RFIDManager& rfid, SensorManager& sensors, GateServoManager& gate, PivotServoManager& pivot) 
    : _rfid(rfid), _sensors(sensors), _gate(gate), _pivot(pivot) {}

void HardwareTest::runFullDiagnostic() {
    Serial.println(F("\n--- DÉBUT DU TEST MATÉRIEL ---"));
    
    runRFIDTest();
    delay(500);
    runIRTest();
    delay(500);
    runGateTest();
    delay(500);
    runPivotTest();

    Serial.println(F("\n--- FIN DU TEST ---"));
}

void HardwareTest::runRFIDTest() {
    Serial.println(F("\n[TEST RFID] : Approchez un badge..."));
    unsigned long t = millis();
    bool success = false;

    while (millis() - t < 5000) {
        if (_rfid.waitForCard()) {
            Serial.printf("  - Badge lu : %s\n", _rfid.getUID().c_str());
            
            _rfid.haltCard(); 
            success = true;
            break;
        }
        yield();
    }

    if (!success) Serial.println(F("  - Erreur : Aucune carte détectée."));
    
    // SOLUTION AU PROBLÈME D'ITÉRATION : 
    // On force une réinitialisation logicielle du module pour la prochaine lecture
    // (Certains modules RC522 se bloquent après une lecture incomplète)
    _rfid.begin(); 
}

void HardwareTest::runIRTest() {
    Serial.println(F("\n[TEST CAPTEURS IR] :"));
    unsigned long t = millis();
    while (millis() - t < 5000) {
        bool in = _sensors.isCarAtEntry();
        bool out = _sensors.isCarAtExit();
        Serial.printf("  - Entrée : %s | Sortie : %s\n", 
                      in ? "Voiture détectée" : "Rien", 
                      out ? "Voiture détectée" : "Rien");
        delay(500);
    }
}

void HardwareTest::runGateTest() {
    Serial.println(F("\n[TEST BARRIÈRE] :"));
    Serial.println(F("  - Mouvement : Ouverture..."));
    _gate.openGate();
    delay(2000);
    Serial.println(F("  - Mouvement : Fermeture..."));
    _gate.closeGate();
}

void HardwareTest::runPivotTest() {
    Serial.println(F("\n[TEST PIVOT] :"));
    Serial.println(F("  - Rotation : Sens Horaire"));
    _pivot.rotateCW();
    delay(2000);
    Serial.println(F("  - Rotation : Sens Anti-horaire"));
    _pivot.rotateCCW();
    delay(2000);
    _pivot.stop();
}