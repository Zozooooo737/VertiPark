#include "RFIDManager.h"
#include <Arduino.h>

RFIDManager::RFIDManager() : mfrc522(PIN_RFID_CS, PIN_RFID_RST) {
    // Initialisation de la clé par défaut (FFAAAAAABBBB etc. en général FF FF FF FF FF FF)
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void RFIDManager::begin() {
    // 1. INITIALISATION DU BUS SPI AVEC TES PINS PERSONNALISÉES
    // SPI.begin(SCK, MISO, MOSI, SS);
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_CS);
    
    // 2. Initialisation du module MFRC522
    mfrc522.PCD_Init();
    
    // 3. Forcer le CS à HAUT pour libérer le bus au démarrage
    pinMode(PIN_RFID_CS, OUTPUT);
    digitalWrite(PIN_RFID_CS, HIGH);
    
    Serial.println(F("RFID: Initialisé avec succès (Bus SPI personnalisé)"));
}

bool RFIDManager::waitForCard() {
    // Vérifie si une nouvelle carte est présente
    if (!mfrc522.PICC_IsNewCardPresent()) return false;
    // Vérifie si on peut lire la carte
    if (!mfrc522.PICC_ReadCardSerial()) return false;
    return true;
}

String RFIDManager::getUID() {
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uidStr += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) uidStr += ":";
    }
    uidStr.toUpperCase();
    return uidStr;
}

void RFIDManager::haltCard() {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

bool RFIDManager::isVertiParkCard() {
    // TODO: Implémenter la lecture du bloc MARQUER_BLOCK
    // Pour l'instant, on simule que toutes les cartes lues sont VertiPark
    // car la logique d'authentification MIFARE est complexe (Key A/B)
    return true; 
}

bool RFIDManager::initVertiParkCard() {
    Serial.println(F("RFID: Marquage de la carte en cours..."));
    // Ici on écrirait "VERTIPARK_CARD" dans le bloc 4
    // Nécessite une authentification réussie sur le secteur
    return true;
}

AccessStatus RFIDManager::checkAccess(String uid) {
    Serial.print(F("RFID: Envoi de l'UID au serveur: "));
    Serial.println(uid);

    // --- SIMULATION SERVEUR ---
    // Cette partie sera remplacée par une requête HTTP POST
    if (uid == "DE:AD:BE:EF") return ACCESS_GRANTED; // Exemple de carte autorisée
    if (uid == "01:02:03:04") return NEED_PAYMENT;
    
    return ACCESS_GRANTED; // Par défaut pour tes tests
}

void RFIDManager::processResponse(AccessStatus status) {
    switch (status) {
        case ACCESS_GRANTED:
            Serial.println(F("RFID: Accès autorisé ! Ouverture barrière."));
            // openBarrier(); -> Appel vers SystemManager ou GateManager
            break;
            
        case ACCESS_DENIED:
            Serial.println(F("RFID: Accès refusé."));
            // denyAccess();
            break;

        case NEED_PAYMENT:
            Serial.println(F("RFID: Paiement requis à la borne."));
            break;

        case UNKNOWN_CARD:
            Serial.println(F("RFID: Carte inconnue."));
            break;
    }
}