#include "HardwareTest.h"

// =============================================================================
// HardwareTest : utilitaire de diagnostic bas niveau (mode DEBUG uniquement)
// Adapte aux nouvelles API non-bloquantes des managers.
// Note : ce fichier conserve volontairement des delay() car il est destine
// au test materiel et n'est utilise QUE si VERTIPARK_DEBUG_MODE est defini.
// =============================================================================

HardwareTest::HardwareTest(RFIDManager& rfid, SensorManager& sensors,
                           GateServoManager& gate, PivotServoManager& pivot,
                           DisplayManager& display, BuzzerManager& buzzer,
                           DHTManager& temp, LedsManager& leds)
    : _rfid(rfid), _sensors(sensors), _gate(gate), _pivot(pivot),
      _display(display), _buzzer(buzzer), _temp(temp), _leds(leds) {}

void HardwareTest::runFullDiagnostic() {
    Serial.println(F("\n--- DEBUT DU TEST MATERIEL ---"));

    runRFIDTest();
    runIRTest();
    runBuzzerTest();
    //runDHTTest();
    runLedsTest();
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

    if (!success) Serial.println(F("  - Erreur : Aucune carte detectee."));
    _rfid.begin();
}

void HardwareTest::runIRTest() {
    Serial.println(F("\n[TEST CAPTEURS IR] :"));
    unsigned long t = millis();
    while (millis() - t < 5000) {
        _sensors.update(millis());
        bool in = _sensors.isCarAtEntry();
        bool out = _sensors.isCarAtExit();
        Serial.printf("  - Entree : %s | Sortie : %s\n",
                      in ? "Voiture detectee" : "Rien",
                      out ? "Voiture detectee" : "Rien");
        delay(500);
    }
}

void HardwareTest::runHallTest() {
    Serial.println(F("\n[TEST CAPTEUR EFFET HALL] :"));
    unsigned long t = millis();
    while (millis() - t < 5000) {
        _sensors.update(millis());
        bool triggered = _sensors.isHallTriggered();
        Serial.printf("  - Capteur HALL : %s\n",
                      triggered ? "Aimant DETECTE" : "Pas d'aimant");
        delay(500);
    }
}

void HardwareTest::runGateTest() {
    Serial.println(F("\n[TEST BARRIERE] :"));
    Serial.println(F("  - Ouverture..."));
    _gate.openGate();
    unsigned long t = millis();
    while (millis() - t < 3000) {
        _gate.update(millis());
        delay(10);
    }
    Serial.println(F("  - Fermeture..."));
    _gate.closeGate();
    t = millis();
    while (millis() - t < 3000) {
        _gate.update(millis());
        delay(10);
    }
}

void HardwareTest::runPivotTest() {
    Serial.println(F("\n[TEST PIVOT] :"));
    Serial.println(F("  - Calibration..."));
    _pivot.startCalibration();
    unsigned long t = millis();
    while (millis() - t < 5000) {
        _pivot.update(millis());
        delay(10);
    }
    _pivot.stopNow();
}

void HardwareTest::runDisplayTest() {
    Serial.println(F("\n[TEST OLED] : Lancement..."));

    _display.startBootAnimation();
    unsigned long t = millis();
    while (millis() - t < 4000) {
        _display.update(millis());
        delay(10);
    }

    _display.showIdle(7);
    delay(2000);

    _display.showNews("14:32", "20/03/2026", "18C", "45%");
    delay(2000);

    _display.showAccessGranted();
    delay(2000);

    _display.showInviteEnter();
    delay(2000);

    _display.showRotation();
    delay(2000);

    for (int i = 0; i <= 100; i += 10) {
        _display.showOpeningBarrier(i);
        delay(50);
    }
    delay(500);

    _display.showInviteAdvanceParking();
    delay(2000);

    for (int i = 0; i <= 100; i += 10) {
        _display.showClosingBarrier(i);
        delay(50);
    }
    delay(500);

    _display.showAskLowerVehicle();
    delay(2000);

    _display.showInviteExit();
    delay(2000);

    _display.showAskEnterVehicle();
    delay(2000);

    _display.showInviteExitVehicle();
    delay(2000);

    _display.showThankYou();
    delay(2000);

    _display.showAccessDenied("Badge inconnu");
    delay(2000);

    _display.showWebsite();
    delay(3000);

    Serial.println(F("[TEST OLED] : Termine."));
}

void HardwareTest::runBuzzerTest() {
    Serial.println(F("\n[TEST BUZZER] :"));
    Serial.println(F("  - SUCCESS"));
    _buzzer.playSuccess();
    unsigned long t = millis();
    while (millis() - t < 800) { _buzzer.update(millis()); delay(5); }

    Serial.println(F("  - ERROR"));
    _buzzer.playError();
    t = millis();
    while (millis() - t < 800) { _buzzer.update(millis()); delay(5); }

    Serial.println(F("  - ALERT"));
    _buzzer.playAlert();
    t = millis();
    while (millis() - t < 1500) { _buzzer.update(millis()); delay(5); }

    _buzzer.stop();
    Serial.println(F("[TEST BUZZER] : Termine."));
}

void HardwareTest::runDHTTest() {
    Serial.println(F("\n[TEST DHT22] :"));
    _temp.forceRefresh();
    _temp.update(millis());
    delay(2000);
    _temp.update(millis());

    float t = _temp.getTemperature();
    float h = _temp.getHumidity();

    if (t > -990.0f) {
        Serial.printf("  - Temperature : %.1f C\n", t);
        Serial.printf("  - Humidite    : %.1f %%\n", h);
    } else {
        Serial.println(F("  - Erreur : Capteur non detecte."));
    }
}

void HardwareTest::runLedsTest() {
    Serial.println(F("\n[TEST DES LEDS DE PLACES] :"));
    _leds.begin();

    Serial.println(F("1. Toutes places libres (VERT)"));
    _leds.setAllPlacesFree();
    delay(2000);

    Serial.println(F("2. Toutes places occupees (ROUGE)"));
    _leds.setAllPlacesOccupied();
    delay(2000);

    Serial.println(F("3. Chenillard place par place"));
    _leds.setAllPlacesFree();
    delay(500);
    for (int i = 0; i < NUM_PLACES; i++) {
        Serial.printf("   -> Place %d OCCUPEE\n", i + 1);
        _leds.setPlaceState(i, PLACE_OCCUPIED);
        delay(800);
    }

    Serial.println(F("\n[TEST LEDS AMBIANCE] :"));

    Serial.println(F("4. BLEUE seule"));
    _leds.setBlueMode(LED_ON);
    _leds.setOrangeMode(LED_OFF);
    delay(2000);

    Serial.println(F("5. ORANGE seule"));
    _leds.setBlueMode(LED_OFF);
    _leds.setOrangeMode(LED_ON);
    delay(2000);

    Serial.println(F("6. Clignotement orange"));
    _leds.setOrangeMode(LED_BLINK_NORMAL);
    unsigned long t = millis();
    while (millis() - t < 3000) {
        _leds.update(millis());
        delay(10);
    }

    _leds.setBlueMode(LED_OFF);
    _leds.setOrangeMode(LED_OFF);
    _leds.setAllPlacesFree();

    Serial.println(F("Fin du test LEDs."));
}
