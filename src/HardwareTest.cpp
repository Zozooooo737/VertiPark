#include "HardwareTest.h"

HardwareTest::HardwareTest(RFIDManager& rfid, SensorManager& sensors, GateServoManager& gate, PivotServoManager& pivot, DisplayManager& display) 
    : _rfid(rfid), _sensors(sensors), _gate(gate), _pivot(pivot), _display(display) {}

void HardwareTest::runFullDiagnostic() {
    Serial.println(F("\n--- DÉBUT DU TEST MATÉRIEL ---"));
    
    runRFIDTest();
    delay(500);
    runIRTest();
    delay(500);
    runGateTest();
    delay(500);
    runPivotTest();
    delay(500);
    runDisplayTest();

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

void HardwareTest::runDisplayTest() {
    Serial.println(F("\n[TEST OLED] : Lancement de la séquence complète..."));

    // 1. Boot et Idle
    _display.oledBootAnimation();
    delay(1000);

    _display.oledIdleScreen(7);
    delay(2000);
    
    _display.oledNewsScreen("14:32", "20/03/2026", "18C", "45%");
    delay(2000);

    // 2. Accès
    _display.oledAccessGranted();
    delay(2000);

    _display.oledInviteEnter();
    delay(2000);

    _display.oledDangerRotation();
    delay(2000);

    // 3. Cycle Barrière Ouverture
    for (int i = 0; i <= 108; i += 6) {
        _display.oledOpeningBarrier(i);
        delay(50);
    }
    delay(500);

    _display.oledInviteAdvanceParking();
    delay(2000);

    // 4. Cycle Barrière Fermeture
    for (int i = 0; i <= 108; i += 6) {
        _display.oledClosingBarrier(i);
        delay(50);
    }
    delay(500);

    // 5. Consignes utilisateur
    _display.oledAskLowerVehicle();
    delay(2000);

    _display.oledInviteExit();
    delay(2000);

    _display.oledAskEnterVehicle();
    delay(2000);

    // 6. Nouveau cycle Barrière (Sortie)
    for (int i = 0; i <= 108; i += 6) {
        _display.oledOpeningBarrier(i);
        delay(50);
    }
    delay(500);

    _display.oledInviteExitVehicle();
    delay(2000);

    for (int i = 0; i <= 108; i += 6) {
        _display.oledClosingBarrier(i);
        delay(50);
    }
    delay(500);

    // 7. Fin et Admin
    _display.oledThankYou();
    delay(2000);

    _display.oledAccessDenied();
    delay(2000);

    _display.oledShowWebsite();
    delay(3000);

    _display.oledAdminWaitCard();
    delay(2000);
    
    _display.oledAdminResetOK();
    delay(2000);

    Serial.println(F("[TEST OLED] : Séquence terminée."));
}