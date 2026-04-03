#pragma once

#include <SPI.h>
#include <MFRC522.h>
#include "config.h"

// Structure demandée pour la cohérence des données
struct RFIDCard {
  String uid;
  bool isVertiParkCard;
};

// Les différents états de réponse du futur serveur
enum AccessStatus {
  ACCESS_GRANTED,
  ACCESS_DENIED,
  NEED_PAYMENT,
  UNKNOWN_CARD
};

class RFIDManager {
private:
    MFRC522 mfrc522;
    MFRC522::MIFARE_Key key;
    
    // Bloc mémoire où on stocke le marqueur "VERTIPARK" (ex: Secteur 1, Bloc 4)
    const byte MARQUER_BLOCK = 4; 
    const String MARQUEUR_TEXT = "VERTIPARK_CARD";

public:
    RFIDManager();
    
    void begin();
    
    // Fonctions de détection et lecture
    bool waitForCard();
    String getUID();
    void haltCard();
    
    // Logique métier
    bool isVertiParkCard();
    bool initVertiParkCard(); // Pour préparer une carte vierge
    
    // Simulation Serveur (En attente de la partie réseau)
    AccessStatus checkAccess(String uid);
    
    // Actions physiques
    void processResponse(AccessStatus status);
};