#include "NetworkManager.h"

// =============================================================================
//                       NETWORK MANAGER - IMPLEMENTATION
// =============================================================================
// Phase 1 : STUB. Tous les hooks sont presents mais inactifs.
// Phase 2 : Definir VERTIPARK_NETWORK_ENABLED dans config.h pour activer.
//
// Pour activer la phase 2, ajouter dans cpp :
//   #include <WiFi.h>
//   #include <HTTPClient.h>
// et implementer le contenu des methodes hookees.
// =============================================================================

#ifdef VERTIPARK_NETWORK_ENABLED
    #include <WiFi.h>
    #include <HTTPClient.h>
#endif

NetworkManager::NetworkManager()
    : _connected(false),
      _lastActionPoll(0),
      _rfidPending(false),
      _rfidReady(false),
      _rfidRequestTime(0),
      _rfidResponse(RFID_RESP_PENDING),
      _rfidAssignedPlace(0) {}

// -----------------------------------------------------------------------------
void NetworkManager::begin() {
    Serial.println(F("[NET] NetworkManager initialise (mode STUB)."));

#ifdef VERTIPARK_NETWORK_ENABLED
    connectWifi();
#else
    // En mode stub : on considere "non connecte" mais le systeme reste fonctionnel
    _connected = false;
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::update(uint32_t now) {

#ifdef VERTIPARK_NETWORK_ENABLED
    // -------- Reconnexion auto si necessaire --------
    if (!_connected && WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Serial.println(F("[NET] WiFi connecte."));
    }
    if (_connected && WiFi.status() != WL_CONNECTED) {
        _connected = false;
        Serial.println(F("[NET] WiFi perdu."));
    }

    // -------- Polling action_parking.txt --------
    if (_connected && (now - _lastActionPoll >= T_NETWORK_POLL_ACTION_MS)) {
        _lastActionPoll = now;
        pollActionFile();
    }

    // -------- Suivi requete RFID --------
    if (_rfidPending) {
        if (now - _rfidRequestTime >= T_NETWORK_HTTP_TIMEOUT_MS) {
            _rfidPending = false;
            _rfidReady = true;
            _rfidResponse = RFID_RESP_TIMEOUT;
        }
        // La completion reelle se fait dans sendRfidCheck()
    }
#else
    (void)now;
    // Mode stub : rien a faire
#endif
}

// -----------------------------------------------------------------------------
bool NetworkManager::isConnected() const {
    return _connected;
}

// -----------------------------------------------------------------------------
bool NetworkManager::consumeAction(ServerAction& outAction) {
    // PHASE 1 : Aucune action recue en mode stub
    // PHASE 2 : Renvoie la derniere action lue depuis action_parking.txt
    outAction.type = ACTION_NONE;
    outAction.placeIndex = 0;
    return false;
}

// -----------------------------------------------------------------------------
void NetworkManager::requestRfidCheck(const String& uid, bool isEntry) {
    _rfidUid = uid;
    _rfidRequestTime = millis();
    _rfidPending = true;
    _rfidReady = false;
    _rfidResponse = RFID_RESP_PENDING;

#if LOG_LEVEL >= 1
    Serial.print(F("[NET] RFID check (stub) UID="));
    Serial.print(uid);
    Serial.print(F(" type="));
    Serial.println(isEntry ? F("ENTRY") : F("EXIT"));
#endif

#ifdef VERTIPARK_NETWORK_ENABLED
    sendRfidCheck();
#else
    // -----------------------------------------------------------------------
    // SIMULATION PHASE 1 :
    // Tous les badges sont acceptes (Q4 valide).
    // Place assignee : pour un test simple, on retourne toujours la place 0.
    // En phase 2, cette logique est remplacee par la reponse JSON du serveur.
    // -----------------------------------------------------------------------
    (void)isEntry;
    _rfidPending = false;
    _rfidReady = true;
    _rfidResponse = RFID_RESP_GRANTED;
    _rfidAssignedPlace = 0;
#endif
}

// -----------------------------------------------------------------------------
bool NetworkManager::isRfidResponseReady() const {
    return _rfidReady;
}

RfidServerResponse NetworkManager::getRfidResponse() const {
    return _rfidResponse;
}

uint8_t NetworkManager::getRfidAssignedPlace() const {
    return _rfidAssignedPlace;
}

void NetworkManager::clearRfidResponse() {
    _rfidReady = false;
    _rfidPending = false;
    _rfidResponse = RFID_RESP_PENDING;
}

// -----------------------------------------------------------------------------
void NetworkManager::notifyPlaceOccupied(uint8_t placeIndex, const String& uid) {
#if LOG_LEVEL >= 1
    Serial.print(F("[NET] Place "));
    Serial.print(placeIndex);
    Serial.print(F(" occupee par "));
    Serial.println(uid);
#endif

#ifdef VERTIPARK_NETWORK_ENABLED
    // PHASE 2 : POST vers API_ENDPOINT_SENSOR avec status=occupee
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::notifyPlaceFreed(uint8_t placeIndex, const String& uid) {
#if LOG_LEVEL >= 1
    Serial.print(F("[NET] Place "));
    Serial.print(placeIndex);
    Serial.print(F(" liberee par "));
    Serial.println(uid);
#endif

#ifdef VERTIPARK_NETWORK_ENABLED
    // PHASE 2 : POST vers API_ENDPOINT_SENSOR avec status=libre
#endif
}

// =============================================================================
//                       HOOKS RESEAU (PHASE 2)
// =============================================================================
// Ces methodes sont des stubs en phase 1. Implementer le contenu lors
// de l'activation backend reelle.
// -----------------------------------------------------------------------------

void NetworkManager::connectWifi() {
#ifdef VERTIPARK_NETWORK_ENABLED
    Serial.print(F("[NET] Connexion WiFi a "));
    Serial.println(F(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // Le suivi de connexion se fait dans update() (non-bloquant)
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::pollActionFile() {
#ifdef VERTIPARK_NETWORK_ENABLED
    // PHASE 2 :
    // HTTPClient http;
    // String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT
    //            + API_BASE_PATH + API_ENDPOINT_ACTION;
    // http.begin(url);
    // int code = http.GET();
    // if (code == 200) {
    //     String payload = http.getString();
    //     // Parser "ENTREE:N" ou "SORTIE:N"
    //     // Stocker dans une queue interne consommee par consumeAction()
    // }
    // http.end();
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::sendRfidCheck() {
#ifdef VERTIPARK_NETWORK_ENABLED
    // PHASE 2 :
    // HTTPClient http;
    // String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT
    //            + API_BASE_PATH + API_ENDPOINT_RFID;
    // http.begin(url);
    // http.addHeader("Content-Type", "application/json");
    // String body = String("{\"uid\":\"") + _rfidUid + "\"}";
    // int code = http.POST(body);
    // if (code == 200) {
    //     // Parser reponse JSON : { "access": true|false, "reason": "...", "place": N }
    //     // Affecter _rfidResponse et _rfidAssignedPlace
    //     // _rfidReady = true; _rfidPending = false;
    // }
    // http.end();
#endif
}
