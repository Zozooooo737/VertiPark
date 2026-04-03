#include <Arduino.h>
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "SystemManager.h"

RFIDManager rfid;
SensorManager sensorManager;
GateServoManager gateManager;
PivotServoManager pivotManager;

SystemManager systemManager(sensorManager, gateManager, pivotManager);


void setup() {
    Serial.begin(115200);
    
    rfid.begin();
    sensorManager.begin();
    gateManager.begin();
    pivotManager.begin();

    systemManager.begin();
}

void loop() {
    //systemManager.process();

    if (rfid.waitForCard()) {
        String currentUID = rfid.getUID();
        
        if (rfid.isVertiParkCard()) {
            AccessStatus result = rfid.checkAccess(currentUID);
            rfid.processResponse(result);
        } else {
            Serial.println("Carte non reconnue par le système VertiPark");
        }
        
        rfid.haltCard(); // Important pour pouvoir lire à nouveau
        delay(1000);     // Anti-rebond simple
    }
}