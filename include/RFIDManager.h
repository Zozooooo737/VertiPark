#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"

// =============================================================================
//                       RFID MANAGER (non-bloquant)
// =============================================================================
// - Scan periodique (T_RFID_POLL_MS) declenche depuis update(now)
// - Anti-rebond : meme UID ignore pendant T_RFID_HALT_DELAY_MS
// - Detection consommable : consumeNewCard() retourne true UNE SEULE FOIS par
//   nouveau badge presente. L'UID est ensuite lisible via getLastUid().
//
// L'autorisation reelle (badge connu / credits OK) est deleguee au
// NetworkManager (cf. cahier des charges).
// =============================================================================

// Conserve pour compat avec l'existant
struct RFIDCard {
    String uid;
    bool isVertiParkCard;
};

enum AccessStatus {
    ACCESS_GRANTED,
    ACCESS_DENIED,
    NEED_PAYMENT,
    UNKNOWN_CARD
};

class RFIDManager {
public:
    RFIDManager();

    void begin();
    void update(uint32_t now);

    // ---- API non-bloquante ----
    // Retourne true UNE SEULE FOIS quand un nouveau badge est detecte.
    bool consumeNewCard();

    // UID du dernier badge lu (vide si aucun)
    String getLastUid() const;

    // Reset interne (force a accepter de nouveau le meme UID)
    void clearLastUid();

    // ---- API bas niveau (compat existant / HardwareTest) ----
    bool waitForCard();
    String getUID();
    void haltCard();
    bool isVertiParkCard();
    bool initVertiParkCard();
    AccessStatus checkAccess(String uid);
    void processResponse(AccessStatus status);

private:
    MFRC522 _mfrc522;
    MFRC522::MIFARE_Key _key;

    // Anti-rebond
    String   _lastUid;
    uint32_t _lastReadTime;
    uint32_t _lastPoll;
    bool     _newCardFlag;

    // Bloc memoire pour le marquage VertiPark
    const byte MARQUER_BLOCK = 4;
    const String MARQUEUR_TEXT = "VERTIPARK_CARD";

    String readUidFromMfrc();
};
