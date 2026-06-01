#pragma once
#include <Arduino.h>
#include "config.h"
#include "RFIDManager.h"
#include "SensorManager.h"
#include "GateServoManager.h"
#include "PivotServoManager.h"
#include "DisplayManager.h"
#include "BuzzerManager.h"
#include "DHTManager.h"
#include "LedsManager.h"
#include "NetworkManager.h"

// =============================================================================
//                       SYSTEM MANAGER (FSM principale)
// =============================================================================
// Orchestre tous les managers via une machine a etats non-bloquante.
//
// CYCLE DE VIE :
//   BOOT -> CALIBRATION (rotation lente jusqu'a Hall, ou bypass si deja la)
//        -> READY (animation succes)
//        -> IDLE (boucle d'attente)
//        -> [ENTRY_*] sur badge RFID acces accorde
//        -> [EXIT_*]  sur commande serveur SORTIE:N (ou RFID en sortie)
//        -> THANKS -> IDLE
//        -> ERROR sur anomalie (auto-recovery)
// =============================================================================

enum SystemState {
    // Initialisation
    SYS_BOOT,
    SYS_CALIBRATION,
    SYS_READY,

    // Veille
    SYS_IDLE,

    // ===== CYCLE ENTREE =====
    SYS_RFID_CHECK,
    SYS_ACCESS_GRANTED,
    SYS_ACCESS_DENIED,

    SYS_ENTRY_ROTATE,
    SYS_ENTRY_INVITE_ENTER,
    SYS_ENTRY_GATE_OPEN,
    SYS_ENTRY_INVITE_ADVANCE,
    SYS_ENTRY_WAIT_CAR_GONE,
    SYS_ENTRY_GATE_CLOSE,
    SYS_ENTRY_INVITE_EXIT_USER,
    SYS_ENTRY_REBALANCE,

    // ===== CYCLE SORTIE =====
    SYS_EXIT_ROTATE_TO_PLACE,
    SYS_EXIT_ASK_ENTER_VEHICLE,
    SYS_EXIT_ASK_WAIT,
    SYS_EXIT_GATE_OPEN,
    SYS_EXIT_INVITE_LEAVE,
    SYS_EXIT_GATE_CLOSE,
    SYS_EXIT_REBALANCE,

    // Final
    SYS_THANKS,

    // Erreur
    SYS_ERROR
};

class SystemManager {
public:
    SystemManager(RFIDManager&       rfid,
                  SensorManager&     sensors,
                  GateServoManager&  gate,
                  PivotServoManager& pivot,
                  DisplayManager&    display,
                  BuzzerManager&     buzzer,
                  DHTManager&        dht,
                  LedsManager&       leds,
                  NetworkManager&    network);

    void begin();
    void update(uint32_t now);

    SystemState getState() const;
    const char* getStateName() const;

private:
    // ---- Refs managers ----
    RFIDManager&       _rfid;
    SensorManager&     _sensors;
    GateServoManager&  _gate;
    PivotServoManager& _pivot;
    DisplayManager&    _display;
    BuzzerManager&     _buzzer;
    DHTManager&        _dht;
    LedsManager&       _leds;
    NetworkManager&    _network;

    // ---- Etat FSM ----
    SystemState _state;
    uint32_t    _stateEntry;
    uint32_t    _lastIdleTick;

    // ---- Donnees cycle courant ----
    String   _currentUid;
    int      _targetPlace;      // 0..NUM_PLACES-1
    bool     _isEntryCycle;     // true = entree, false = sortie
    String   _denyReason;

    // ---- Transition ----
    void transitionTo(SystemState next);
    bool stateElapsed(uint32_t ms) const;

    // ---- Sync LED/Buzzer (commun a rotation et fermeture/ouverture) ----
    void syncOrangeBlinkWithBuzzer();

    // ---- Equilibrage carrousel ----
    // Renvoie le numero de place vers laquelle se positionner pour equilibrer
    // les places occupees (logique strictement locale).
    int  computeBalancedPlace() const;

    // ---- Helpers d'orchestration ----
    void enterIdleMode();
    void enterErrorMode(const char* title, const char* msg);

    // ---- Handlers d'etats ----
    void handleBoot(uint32_t now);
    void handleCalibration(uint32_t now);
    void handleReady(uint32_t now);
    void handleIdle(uint32_t now);

    void handleRfidCheck(uint32_t now);
    void handleAccessGranted(uint32_t now);
    void handleAccessDenied(uint32_t now);

    void handleEntryRotate(uint32_t now);
    void handleEntryInviteEnter(uint32_t now);
    void handleEntryGateOpen(uint32_t now);
    void handleEntryInviteAdvance(uint32_t now);
    void handleEntryWaitCarGone(uint32_t now);
    void handleEntryGateClose(uint32_t now);
    void handleEntryInviteExitUser(uint32_t now);
    void handleEntryRebalance(uint32_t now);

    void handleExitRotateToPlace(uint32_t now);
    void handleExitAskEnterVehicle(uint32_t now);
    void handleExitAskWait(uint32_t now);
    void handleExitGateOpen(uint32_t now);
    void handleExitInviteLeave(uint32_t now);
    void handleExitGateClose(uint32_t now);
    void handleExitRebalance(uint32_t now);

    void handleThanks(uint32_t now);
    void handleError(uint32_t now);
};
