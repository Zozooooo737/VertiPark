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
LedsManager leds;

HardwareTest tester(rfid, sensors, gate, pivot, display, buzzer, temp, leds);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("--- DEMARRAGE DU VERTIPARK ---");

    // 1. ÉTAPE CRUCIALE : On prépare les broches CS pour éviter que les deux puces parlent en même temps
    pinMode(OLED_CS, OUTPUT);
    pinMode(PIN_RFID_CS, OUTPUT);
    digitalWrite(OLED_CS, HIGH);       // Écran mis en attente
    digitalWrite(PIN_RFID_CS, HIGH);   // RFID mis en attente

    // 2. INITIALISATION UNIQUE DU SPI : On déclare TOUTES les broches, surtout le MISO pour le RFID !
    // On part du principe que tes pins dans config.h sont SCK=14, MISO=12 (ou la broche associée), MOSI=13
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, OLED_CS); 

    Serial.println("Initialisation des managers...");
    
    // 3. INITIALISATION DE L'ÉCRAN
    display.begin();
    display.oledBootAnimation(); 
    delay(200);

    // 4. INITIALISATION DU RFID 
    // Maintenant qu'il a sa broche MISO active, il va pouvoir lire les badges !
    rfid.begin();

    // 5. Reste des managers
    gate.begin();
    pivot.begin();
    sensors.begin();
    buzzer.begin();
    temp.begin();
    leds.begin();
    
    Serial.println("Systeme pret !");
}

void loop() {
    tester.runFullDiagnostic();
    delay(2000);
}