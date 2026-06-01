#include <Arduino.h>
#include <SPI.h>
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
// ==== Managers ====
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "DisplayManager.h"
#include "BuzzerManager.h"
#include "DHTManager.h"
#include "LedsManager.h"
#include "NetworkManager.h"
#include "SystemManager.h"

// ==== HardwareTest uniquement si mode DEBUG ====
#ifdef VERTIPARK_DEBUG_MODE
    #include "HardwareTest.h"
#endif

// =============================================================================
//                       INSTANCES GLOBALES
// =============================================================================
RFIDManager       rfid;
SensorManager     sensors;
GateServoManager  gate;
PivotServoManager pivot;
DisplayManager    display;
BuzzerManager     buzzer;
DHTManager        dht;
LedsManager       leds;
NetworkManager    network;

SystemManager     systemMgr(rfid, sensors, gate, pivot,
                            display, buzzer, dht, leds, network);

#ifdef VERTIPARK_DEBUG_MODE
HardwareTest tester(rfid, sensors, gate, pivot, display, buzzer, dht, leds);
#endif

// =============================================================================
//                              SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println();
    Serial.println(F("================================================="));
    Serial.println(F("   VERTIPARK - Demarrage"));
    Serial.println(F("================================================="));

    // ---- 1) Preparation du bus SPI partage (OLED + RFID) ----
    pinMode(OLED_CS, OUTPUT);
    pinMode(PIN_RFID_CS, OUTPUT);
    digitalWrite(OLED_CS, HIGH);
    digitalWrite(PIN_RFID_CS, HIGH);

    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_CS);

    // ---- 2) Initialisation des managers ----
    display.begin();
    //rfid.begin();
    gate.begin();
    pivot.begin();
    sensors.begin();
    buzzer.begin();
    dht.begin();
    leds.begin();
    network.begin();

#ifdef VERTIPARK_DEBUG_MODE
    Serial.println(F("[MAIN] MODE DEBUG - HardwareTest"));
#else
    // ---- 3) Demarrage de la FSM principale ----
    systemMgr.begin();
#endif

    Serial.println(F("[MAIN] Systeme initialise."));
}

// =============================================================================
//                               LOOP
// =============================================================================
void loop() {
    const uint32_t now = millis();

    // ---- Mise a jour de tous les managers (non-bloquant) ----
    sensors.update(now);
    dht.update(now);
    rfid.update(now);
    network.update(now);

    gate.update(now);
    pivot.update(now);
    buzzer.update(now);
    leds.update(now);
    display.update(now);

#ifdef VERTIPARK_DEBUG_MODE
    // ---- Mode DEBUG : test materiel periodique ----
    static uint32_t lastDiag = 0;
    if (now - lastDiag >= 2000) {
        lastDiag = now;
        tester.runFullDiagnostic();
    }
#else
    // ---- Mode PRODUCTION : FSM principale ----
    systemMgr.update(now);
#endif
}
