#include "NetworkManager.h"

// =============================================================================
//                        NETWORK MANAGER - IMPLEMENTATION
// =============================================================================

#ifdef VERTIPARK_NETWORK_ENABLED
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <ArduinoJson.h> // INDISPENSABLE POUR LA PHASE 2
#endif

// Variables statiques pour stocker l'ordre web (évite de modifier le .h)
#ifdef VERTIPARK_NETWORK_ENABLED
    static ServerAction pendingWebAction = {ACTION_NONE, 0};
    static bool hasPendingAction = false;
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
    Serial.println(F("[NET] NetworkManager initialise."));

#ifdef VERTIPARK_NETWORK_ENABLED
    Serial.println(F("[NET] Mode PRODUCTION (Reseau active)"));
    connectWifi();
#else
    Serial.println(F("[NET] Mode STUB (Reseau desactive)"));
    _connected = false;
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::update(uint32_t now) {

#ifdef VERTIPARK_NETWORK_ENABLED
    // -------- Reconnexion auto si necessaire --------
    if (!_connected && WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Serial.println(F("[NET] WiFi connecte ! IP: ") + WiFi.localIP().toString());
    }
    if (_connected && WiFi.status() != WL_CONNECTED) {
        _connected = false;
        Serial.println(F("[NET] WiFi perdu. Tentative de reconnexion..."));
        WiFi.reconnect();
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
            Serial.println(F("[NET] Erreur : Timeout RFID"));
        }
    }
#else
    (void)now; // Mode stub : rien a faire
#endif
}

// -----------------------------------------------------------------------------
bool NetworkManager::isConnected() const {
    return _connected;
}

// -----------------------------------------------------------------------------
bool NetworkManager::consumeAction(ServerAction& outAction) {
#ifdef VERTIPARK_NETWORK_ENABLED
    // PHASE 2 : Renvoie la derniere action lue
    if (hasPendingAction) {
        outAction = pendingWebAction;
        hasPendingAction = false;            // On consomme l'action
        pendingWebAction.type = ACTION_NONE; // On la reset
        return true;
    }
#endif

    // PHASE 1 ou pas d'action : 
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
    Serial.print(F("[NET] RFID check UID="));
    Serial.print(uid);
    Serial.print(F(" type="));
    Serial.println(isEntry ? F("ENTRY") : F("EXIT"));
#endif

#ifdef VERTIPARK_NETWORK_ENABLED
    sendRfidCheck();
#else
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
    if (!_connected) return;
    HTTPClient http;
    String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT + API_BASE_PATH + "/update_sensor.php";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["numero"] = placeIndex;
    doc["etat"] = "occupee";
    String body;
    serializeJson(doc, body);

    http.POST(body);
    http.end();
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
    if (!_connected) return;
    HTTPClient http;
    String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT + API_BASE_PATH + "/update_sensor.php";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["numero"] = placeIndex;
    doc["etat"] = "libre";
    String body;
    serializeJson(doc, body);

    http.POST(body);
    http.end();
#endif
}

// =============================================================================
//                        HOOKS RESEAU (PHASE 2)
// =============================================================================

void NetworkManager::connectWifi() {
#ifdef VERTIPARK_NETWORK_ENABLED
    Serial.print(F("[NET] Connexion WiFi a "));
    Serial.println(F(WIFI_SSID));
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::pollActionFile() {
#ifdef VERTIPARK_NETWORK_ENABLED
    HTTPClient http;
    // On cible lire_action.php
    String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT + API_BASE_PATH + "/lire_action.php";
    
    http.begin(url);
    int code = http.GET();
    
    if (code == 200) {
        String payload = http.getString();
        payload.trim();
        
        if (payload != "RIEN" && payload != "") {
            Serial.println("[NET] Ordre Web recu: " + payload);
            
            // Parsing de "ENTREE:2" ou "SORTIE:1"
            if (payload.startsWith("ENTREE:")) {
                pendingWebAction.type = ACTION_ENTREE; 
                pendingWebAction.placeIndex = payload.substring(7).toInt();
                hasPendingAction = true;
            } 
            else if (payload.startsWith("SORTIE:")) {
                pendingWebAction.type = ACTION_SORTIE;
                pendingWebAction.placeIndex = payload.substring(7).toInt();
                hasPendingAction = true;
            }
        }
    }
    http.end();
#endif
}

// -----------------------------------------------------------------------------
void NetworkManager::sendRfidCheck() {
#ifdef VERTIPARK_NETWORK_ENABLED
    HTTPClient http;
    String url = String("http://") + API_SERVER_HOST + ":" + API_SERVER_PORT + API_BASE_PATH + "/rfid_check.php";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    // Preparation du JSON pour le serveur
    StaticJsonDocument<200> doc;
    doc["uid_rfid"] = _rfidUid;
    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    
    if (code == 200) {
        String response = http.getString();
        StaticJsonDocument<300> respDoc;
        DeserializationError error = deserializeJson(respDoc, response);

        if (!error) {
            if (respDoc["access"] == true) {
                _rfidResponse = RFID_RESP_GRANTED;
                // S'assure que PHP renvoie bien un numero de place dans "place"
                _rfidAssignedPlace = respDoc["place_num"] | 0;
            } else {
                _rfidResponse = RFID_RESP_DENIED;
            }
        } else {
            _rfidResponse = RFID_RESP_ERROR;
        }
    } else {
        _rfidResponse = RFID_RESP_TIMEOUT;
    }
    
    _rfidReady = true;
    _rfidPending = false;
    http.end();
#endif
}
