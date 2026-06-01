#pragma once
#include <Arduino.h>
#include "config.h"

// =============================================================================
//                          NETWORK MANAGER (STUB)
// =============================================================================
// PHASE 1 : Architecture complete + hooks simules
// PHASE 2 : Activer VERTIPARK_NETWORK_ENABLED dans config.h pour brancher
//           WiFi + HTTPClient sur le backend PHP reel.
//
// Endpoints utilises (definis dans config.h) :
//   - API_ENDPOINT_ACTION : GET   commandes ENTREE:N / SORTIE:N
//   - API_ENDPOINT_RFID   : POST  validation UID badge
//   - API_ENDPOINT_STATUS : GET   etat global des places
//   - API_ENDPOINT_SENSOR : POST  synchronisation capteurs
// =============================================================================

// Types de commandes recues du serveur
enum ServerActionType {
    ACTION_NONE,
    ACTION_ENTRY,       // ENTREE:N
    ACTION_EXIT         // SORTIE:N
};

// Reponse serveur RFID
enum RfidServerResponse {
    RFID_RESP_PENDING,          // En attente
    RFID_RESP_GRANTED,          // Acces autorise (badge ok + credits ok)
    RFID_RESP_DENIED_UNKNOWN,   // Badge inconnu
    RFID_RESP_DENIED_NO_CREDIT, // Solde insuffisant
    RFID_RESP_TIMEOUT           // Timeout serveur
};

struct ServerAction {
    ServerActionType type;
    uint8_t placeIndex;     // 0..NUM_PLACES-1
};

class NetworkManager {
public:
    NetworkManager();

    void begin();
    void update(uint32_t now);

    // ---- WiFi ----
    bool isConnected() const;

    // ---- Polling actions serveur (action_parking.txt) ----
    // Retourne true si une nouvelle action est disponible.
    // L'action est consommee par cet appel (vidage equivalent).
    bool consumeAction(ServerAction& outAction);

    // ---- Validation RFID (asynchrone) ----
    // Lance une requete vers rfid_check.php avec l'UID donne.
    // Le resultat est recupere via getRfidResponse() / isRfidResponseReady().
    void requestRfidCheck(const String& uid, bool isEntry);
    bool isRfidResponseReady() const;
    RfidServerResponse getRfidResponse() const;
    uint8_t getRfidAssignedPlace() const;   // Place assignee si granted

    // ---- Notification etat place ----
    void notifyPlaceOccupied(uint8_t placeIndex, const String& uid);
    void notifyPlaceFreed(uint8_t placeIndex, const String& uid);

    // ---- Reset etat interne ----
    void clearRfidResponse();

private:
    bool _connected;
    uint32_t _lastActionPoll;

    // Etat reponse RFID
    bool _rfidPending;
    bool _rfidReady;
    uint32_t _rfidRequestTime;
    RfidServerResponse _rfidResponse;
    uint8_t _rfidAssignedPlace;
    String _rfidUid;

    // ---- Hooks reseau reels (phase 2) ----
    // Ces methodes seront implementees lorsque VERTIPARK_NETWORK_ENABLED sera defini.
    void connectWifi();
    void pollActionFile();
    void sendRfidCheck();
};
