#include <Arduino.h>
#include "config.h"
#include "HardwareTest.h"

RFIDManager rfid;
SensorManager sensors;
GateServoManager gate;
PivotServoManager pivot;
DisplayManager display;
BuzzerManager buzzer;
DHTManager temp;

HardwareTest tester(rfid, sensors, gate, pivot, display, buzzer, temp);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initialisation du bus SPI...");

    // 1. Désactive les CS immédiatement avant toute chose
    pinMode(PIN_RFID_CS, OUTPUT);
    pinMode(OLED_CS, OUTPUT);
    digitalWrite(PIN_RFID_CS, HIGH); 
    digitalWrite(OLED_CS, HIGH);

    // 2. Initialise le SPI
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI); 

    // 3. Lance les managers
    Serial.println("Demarrage Managers...");
    display.begin();
    delay(100);
    rfid.begin();
    gate.begin();
    pivot.begin();
    sensors.begin();
    buzzer.begin();
    temp.begin();
    Serial.println("Systeme pret !");
}

void loop() {
    tester.runFullDiagnostic();
    
    delay(2000);
}