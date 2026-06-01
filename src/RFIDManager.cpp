#include "RFIDManager.h"

// =============================================================================
//             RFID MANAGER - IMPLEMENTATION (non-bloquant)
// =============================================================================

RFIDManager::RFIDManager()
    : _mfrc522(PIN_RFID_CS, PIN_RFID_RST),
      _lastReadTime(0),
      _lastPoll(0),
      _newCardFlag(false) {
    for (byte i = 0; i < 6; i++) _key.keyByte[i] = 0xFF;
}

// -----------------------------------------------------------------------------
void RFIDManager::begin() {
    // CRITIQUE : ce second SPI.begin() est INDISPENSABLE pour la cohabitation
    // OLED + MFRC522 sur le meme bus SPI partage. Sequence empiriquement validee.
    // Ne PAS supprimer - meme si SPI.begin() est deja appele dans main.cpp.
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI, PIN_RFID_CS);

    _mfrc522.PCD_Init();

    pinMode(PIN_RFID_CS, OUTPUT);
    digitalWrite(PIN_RFID_CS, HIGH);

#if LOG_LEVEL >= 1
    Serial.println(F("[RFID] Initialise."));
#endif
}

// -----------------------------------------------------------------------------
void RFIDManager::update(uint32_t now) {
    if (now - _lastPoll < T_RFID_POLL_MS) return;
    _lastPoll = now;

    if (!_mfrc522.PICC_IsNewCardPresent()) return;
    if (!_mfrc522.PICC_ReadCardSerial())   return;

    String uid = readUidFromMfrc();
    _mfrc522.PICC_HaltA();

    // Anti-rebond : meme UID ignore pendant T_RFID_HALT_DELAY_MS
    if (uid == _lastUid && (now - _lastReadTime) < T_RFID_HALT_DELAY_MS) {
        return;
    }

    _lastUid = uid;
    _lastReadTime = now;
    _newCardFlag = true;

#if LOG_LEVEL >= 1
    Serial.print(F("[RFID] Badge detecte : "));
    Serial.println(uid);
#endif
}

// =============================================================================
//                       API NON-BLOQUANTE
// =============================================================================

bool RFIDManager::consumeNewCard() {
    if (_newCardFlag) {
        _newCardFlag = false;
        return true;
    }
    return false;
}

String RFIDManager::getLastUid() const {
    return _lastUid;
}

void RFIDManager::clearLastUid() {
    _lastUid = "";
    _lastReadTime = 0;
    _newCardFlag = false;
}

// =============================================================================
//                       LECTURE UID BAS NIVEAU
// =============================================================================

String RFIDManager::readUidFromMfrc() {
    String uidStr = "";
    for (byte i = 0; i < _mfrc522.uid.size; i++) {
        uidStr += String(_mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uidStr += String(_mfrc522.uid.uidByte[i], HEX);
        if (i < _mfrc522.uid.size - 1) uidStr += ":";
    }
    uidStr.toUpperCase();
    return uidStr;
}

// =============================================================================
//             API BAS NIVEAU (compatibilite HardwareTest)
// =============================================================================

bool RFIDManager::waitForCard() {
    if (!_mfrc522.PICC_IsNewCardPresent()) return false;
    if (!_mfrc522.PICC_ReadCardSerial())   return false;
    return true;
}

String RFIDManager::getUID() {
    return readUidFromMfrc();
}

void RFIDManager::haltCard() {
    _mfrc522.PICC_HaltA();
    _mfrc522.PCD_StopCrypto1();
}

bool RFIDManager::isVertiParkCard() {
    // En phase 1 : tous les badges sont consideres VertiPark
    return true;
}

bool RFIDManager::initVertiParkCard() {
#if LOG_LEVEL >= 1
    Serial.println(F("[RFID] Marquage carte (stub)"));
#endif
    return true;
}

AccessStatus RFIDManager::checkAccess(String uid) {
    // La validation reelle se fait via NetworkManager.requestRfidCheck()
    // Cette methode reste pour compat HardwareTest uniquement.
    if (uid == "DE:AD:BE:EF") return ACCESS_GRANTED;
    if (uid == "01:02:03:04") return NEED_PAYMENT;
    return ACCESS_GRANTED;
}

void RFIDManager::processResponse(AccessStatus status) {
#if LOG_LEVEL >= 1
    switch (status) {
        case ACCESS_GRANTED: Serial.println(F("[RFID] Acces autorise.")); break;
        case ACCESS_DENIED:  Serial.println(F("[RFID] Acces refuse."));   break;
        case NEED_PAYMENT:   Serial.println(F("[RFID] Paiement requis.")); break;
        case UNKNOWN_CARD:   Serial.println(F("[RFID] Carte inconnue.")); break;
    }
#else
    (void)status;
#endif
}
