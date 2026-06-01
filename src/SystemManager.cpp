#include "SystemManager.h"

// =============================================================================
//             SYSTEM MANAGER - IMPLEMENTATION (FSM principale)
// =============================================================================

SystemManager::SystemManager(RFIDManager&       rfid,
                             SensorManager&     sensors,
                             GateServoManager&  gate,
                             PivotServoManager& pivot,
                             DisplayManager&    display,
                             BuzzerManager&     buzzer,
                             DHTManager&        dht,
                             LedsManager&       leds,
                             NetworkManager&    network)
    : _rfid(rfid), _sensors(sensors), _gate(gate), _pivot(pivot),
      _display(display), _buzzer(buzzer), _dht(dht), _leds(leds),
      _network(network),
      _state(SYS_BOOT), _stateEntry(0), _lastIdleTick(0),
      _targetPlace(0), _isEntryCycle(true) {}

// -----------------------------------------------------------------------------
void SystemManager::begin() {
    _stateEntry = millis();
    _state = SYS_BOOT;
    _display.startBootAnimation();
    _leds.setOrangeMode(LED_BLINK_NORMAL);
    _leds.setBlueMode(LED_OFF);
#if LOG_LEVEL >= 1
    Serial.println(F("[SYS] BOOT"));
#endif
}

// -----------------------------------------------------------------------------
void SystemManager::update(uint32_t now) {
    switch (_state) {
        case SYS_BOOT:                       handleBoot(now);                  break;
        case SYS_CALIBRATION:                handleCalibration(now);           break;
        case SYS_READY:                      handleReady(now);                 break;
        case SYS_IDLE:                       handleIdle(now);                  break;
        case SYS_RFID_CHECK:                 handleRfidCheck(now);             break;
        case SYS_ACCESS_GRANTED:             handleAccessGranted(now);         break;
        case SYS_ACCESS_DENIED:              handleAccessDenied(now);          break;
        case SYS_ENTRY_ROTATE:               handleEntryRotate(now);           break;
        case SYS_ENTRY_INVITE_ENTER:         handleEntryInviteEnter(now);      break;
        case SYS_ENTRY_GATE_OPEN:            handleEntryGateOpen(now);         break;
        case SYS_ENTRY_INVITE_ADVANCE:       handleEntryInviteAdvance(now);    break;
        case SYS_ENTRY_WAIT_CAR_GONE:        handleEntryWaitCarGone(now);      break;
        case SYS_ENTRY_GATE_CLOSE:           handleEntryGateClose(now);        break;
        case SYS_ENTRY_INVITE_EXIT_USER:     handleEntryInviteExitUser(now);   break;
        case SYS_ENTRY_REBALANCE:            handleEntryRebalance(now);        break;
        case SYS_EXIT_ROTATE_TO_PLACE:       handleExitRotateToPlace(now);     break;
        case SYS_EXIT_ASK_ENTER_VEHICLE:     handleExitAskEnterVehicle(now);   break;
        case SYS_EXIT_ASK_WAIT:              handleExitAskWait(now);           break;
        case SYS_EXIT_GATE_OPEN:             handleExitGateOpen(now);          break;
        case SYS_EXIT_INVITE_LEAVE:          handleExitInviteLeave(now);       break;
        case SYS_EXIT_GATE_CLOSE:            handleExitGateClose(now);         break;
        case SYS_EXIT_REBALANCE:             handleExitRebalance(now);         break;
        case SYS_THANKS:                     handleThanks(now);                break;
        case SYS_ERROR:                      handleError(now);                 break;
    }

    // Sync LED orange avec buzzer (mode EXTERN_SYNC)
    syncOrangeBlinkWithBuzzer();
}

// =============================================================================
//                       UTILITAIRES FSM
// =============================================================================

void SystemManager::transitionTo(SystemState next) {
    _state = next;
    _stateEntry = millis();
#if LOG_LEVEL >= 1
    Serial.print(F("[SYS] -> "));
    Serial.println(getStateName());
#endif
}

bool SystemManager::stateElapsed(uint32_t ms) const {
    return (millis() - _stateEntry) >= ms;
}

void SystemManager::syncOrangeBlinkWithBuzzer() {
    if (_buzzer.consumeSyncTick()) {
        if (_leds.getOrangeMode() == LED_EXTERN_SYNC) {
            _leds.toggleOrange();
        }
    }
}

SystemState SystemManager::getState() const { return _state; }

const char* SystemManager::getStateName() const {
    switch (_state) {
        case SYS_BOOT:                   return "BOOT";
        case SYS_CALIBRATION:            return "CALIBRATION";
        case SYS_READY:                  return "READY";
        case SYS_IDLE:                   return "IDLE";
        case SYS_RFID_CHECK:             return "RFID_CHECK";
        case SYS_ACCESS_GRANTED:         return "ACCESS_GRANTED";
        case SYS_ACCESS_DENIED:          return "ACCESS_DENIED";
        case SYS_ENTRY_ROTATE:           return "ENTRY_ROTATE";
        case SYS_ENTRY_INVITE_ENTER:     return "ENTRY_INVITE_ENTER";
        case SYS_ENTRY_GATE_OPEN:        return "ENTRY_GATE_OPEN";
        case SYS_ENTRY_INVITE_ADVANCE:   return "ENTRY_INVITE_ADVANCE";
        case SYS_ENTRY_WAIT_CAR_GONE:    return "ENTRY_WAIT_CAR_GONE";
        case SYS_ENTRY_GATE_CLOSE:       return "ENTRY_GATE_CLOSE";
        case SYS_ENTRY_INVITE_EXIT_USER: return "ENTRY_INVITE_EXIT_USER";
        case SYS_ENTRY_REBALANCE:        return "ENTRY_REBALANCE";
        case SYS_EXIT_ROTATE_TO_PLACE:   return "EXIT_ROTATE_TO_PLACE";
        case SYS_EXIT_ASK_ENTER_VEHICLE: return "EXIT_ASK_ENTER_VEHICLE";
        case SYS_EXIT_ASK_WAIT:          return "EXIT_ASK_WAIT";
        case SYS_EXIT_GATE_OPEN:         return "EXIT_GATE_OPEN";
        case SYS_EXIT_INVITE_LEAVE:      return "EXIT_INVITE_LEAVE";
        case SYS_EXIT_GATE_CLOSE:        return "EXIT_GATE_CLOSE";
        case SYS_EXIT_REBALANCE:         return "EXIT_REBALANCE";
        case SYS_THANKS:                 return "THANKS";
        case SYS_ERROR:                  return "ERROR";
        default:                         return "UNKNOWN";
    }
}

// =============================================================================
//                       BOOT / CALIBRATION / READY
// =============================================================================

void SystemManager::handleBoot(uint32_t now) {
    if (_display.isAnimationDone()) {
        // Q9 : Si Hall deja actif au boot -> bypass calibration
        if (_sensors.isHallTriggered()) {
            _pivot.notifyHallTrigger();
            _display.startReadyAnimation();
            _buzzer.playSuccess();
            _leds.setOrangeMode(LED_OFF);
            _leds.setBlueMode(LED_BLINK_NORMAL);
            transitionTo(SYS_READY);
            return;
        }

        // Sinon : calibration douce
        _display.startCalibrationAnimation();
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        _pivot.startCalibration();
        transitionTo(SYS_CALIBRATION);
    }
}

void SystemManager::handleCalibration(uint32_t now) {
    if (stateElapsed(T_CALIBRATION_TIMEOUT_MS)) {
        _pivot.stopNow();
        _buzzer.stop();
        enterErrorMode("CALIB", "Hall non detecte");
        return;
    }

    if (_sensors.consumeHallRising()) {
        _pivot.notifyHallTrigger();
        _buzzer.stop();
        _leds.setOrangeMode(LED_OFF);

        _display.startReadyAnimation();
        _buzzer.playSuccess();
        _leds.setBlueMode(LED_BLINK_NORMAL);
        transitionTo(SYS_READY);
    }

    if (_pivot.hasError()) {
        _buzzer.stop();
        enterErrorMode("PIVOT", "Erreur calibration");
    }
}

void SystemManager::handleReady(uint32_t now) {
    if (_display.isAnimationDone() && stateElapsed(T_READY_ANIM_MS)) {
        enterIdleMode();
    }
}

// =============================================================================
//                              IDLE
// =============================================================================

void SystemManager::enterIdleMode() {
    int libres = 0;
    for (int i = 0; i < NUM_PLACES; i++) {
        if (_leds.getPlaceState(i) == PLACE_FREE) libres++;
    }
    _display.setIdleData(libres, _dht.getTemperature(), _dht.getHumidity());
    _display.setMode(SCREEN_IDLE_CYCLE);

    _leds.setBlueMode(LED_ON);
    _leds.setOrangeMode(LED_ON);
    _buzzer.stop();

    transitionTo(SYS_IDLE);
}

void SystemManager::handleIdle(uint32_t now) {
    // Mise a jour DHT + places toutes les secondes
    if (now - _lastIdleTick >= 1000) {
        _lastIdleTick = now;
        int libres = 0;
        for (int i = 0; i < NUM_PLACES; i++) {
            if (_leds.getPlaceState(i) == PLACE_FREE) libres++;
        }
        _display.setIdleData(libres, _dht.getTemperature(), _dht.getHumidity());
    }

    // Detection badge RFID
    if (_rfid.consumeNewCard()) {
        _currentUid = _rfid.getLastUid();
        _isEntryCycle = true;
        _network.requestRfidCheck(_currentUid, true);
        _display.showError("VERIFICATION", "Lecture serveur...");
        transitionTo(SYS_RFID_CHECK);
        return;
    }

    // Commande serveur (action_parking.txt)
    ServerAction act;
    if (_network.consumeAction(act)) {
        if (act.type == ACTION_ENTRY) {
            _targetPlace = act.placeIndex;
            _isEntryCycle = true;
            _currentUid = "WEB_USER";
            transitionTo(SYS_ACCESS_GRANTED);
        } else if (act.type == ACTION_EXIT) {
            _targetPlace = act.placeIndex;
            _isEntryCycle = false;
            _currentUid = "WEB_USER";
            transitionTo(SYS_EXIT_ROTATE_TO_PLACE);
        }
    }
}

// =============================================================================
//                       VERIFICATION RFID
// =============================================================================

void SystemManager::handleRfidCheck(uint32_t now) {
    if (stateElapsed(T_RFID_RESPONSE_TIMEOUT_MS)) {
        _denyReason = "Serveur HS";
        transitionTo(SYS_ACCESS_DENIED);
        return;
    }

    if (!_network.isRfidResponseReady()) return;

    RfidServerResponse resp = _network.getRfidResponse();
    _network.clearRfidResponse();

    switch (resp) {
        case RFID_RESP_GRANTED:
            _targetPlace = _network.getRfidAssignedPlace();
            transitionTo(SYS_ACCESS_GRANTED);
            break;
        case RFID_RESP_DENIED_NO_CREDIT:
            _denyReason = "Solde insuffisant";
            transitionTo(SYS_ACCESS_DENIED);
            break;
        case RFID_RESP_DENIED_UNKNOWN:
            _denyReason = "Badge inconnu";
            transitionTo(SYS_ACCESS_DENIED);
            break;
        case RFID_RESP_TIMEOUT:
        default:
            _denyReason = "Erreur serveur";
            transitionTo(SYS_ACCESS_DENIED);
            break;
    }
}

void SystemManager::handleAccessGranted(uint32_t now) {
    if (now == _stateEntry || stateElapsed(0) && _display.getMode() != SCREEN_STATIC) {
        _display.showAccessGranted();
        _buzzer.playSuccess();
        _leds.setBlueMode(LED_BLINK_FAST);
        _leds.setOrangeMode(LED_OFF);
    }
    if (stateElapsed(T_ACCESS_GRANTED_DISPLAY_MS)) {
        transitionTo(SYS_ENTRY_ROTATE);
    }
}

void SystemManager::handleAccessDenied(uint32_t now) {
    if (now == _stateEntry || _display.getMode() != SCREEN_STATIC) {
        _display.showAccessDenied(_denyReason.c_str());
        _buzzer.playError();
        _leds.setBlueMode(LED_OFF);
        _leds.setOrangeMode(LED_BLINK_FAST);
    }
    if (stateElapsed(T_ACCESS_DENIED_DISPLAY_MS)) {
        enterIdleMode();
    }
}

// =============================================================================
//                       CYCLE ENTREE
// =============================================================================

void SystemManager::handleEntryRotate(uint32_t now) {
    if (now == _stateEntry) {
        _display.showRotation();
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        _leds.setBlueMode(LED_OFF);
        _pivot.goToPlace(_targetPlace);
    }

    if (_pivot.hasError()) {
        _buzzer.stop();
        enterErrorMode("PIVOT", "Rotation impossible");
        return;
    }

    if (!_pivot.isBusy() && _pivot.isAtTarget()) {
        _buzzer.stop();
        _leds.setOrangeMode(LED_ON);
        transitionTo(SYS_ENTRY_INVITE_ENTER);
    }
}

void SystemManager::handleEntryInviteEnter(uint32_t now) {
    if (now == _stateEntry) {
        _display.showInviteEnter();
        _leds.setOrangeMode(LED_ON);
    }

    if (_sensors.consumeIrEntryRising()) {
        _gate.openGate();
        _display.showOpeningBarrier(0);
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        transitionTo(SYS_ENTRY_GATE_OPEN);
        return;
    }

    if (stateElapsed(T_WAIT_CAR_AT_ENTRY_MS)) {
        enterIdleMode();
    }
}

void SystemManager::handleEntryGateOpen(uint32_t now) {
    _display.showOpeningBarrier(_gate.getProgress());

    if (!_gate.isMoving() && _gate.isOpen()) {
        _buzzer.stop();
        _leds.setOrangeMode(LED_ON);
        transitionTo(SYS_ENTRY_INVITE_ADVANCE);
    }
}

void SystemManager::handleEntryInviteAdvance(uint32_t now) {
    if (now == _stateEntry) {
        _display.showInviteAdvanceParking();
    }
    if (!_sensors.isCarAtEntry()) {
        _sensors.consumeIrEntryFalling();
        transitionTo(SYS_ENTRY_WAIT_CAR_GONE);
        return;
    }
    if (stateElapsed(T_GATE_HOLD_OPEN_MS)) {
        transitionTo(SYS_ENTRY_WAIT_CAR_GONE);
    }
}

void SystemManager::handleEntryWaitCarGone(uint32_t now) {
    if (stateElapsed(T_AFTER_PASS_BEFORE_CLOSE_MS)) {
        _gate.closeGate();
        _display.showClosingBarrier(0);
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        transitionTo(SYS_ENTRY_GATE_CLOSE);
    }
}

void SystemManager::handleEntryGateClose(uint32_t now) {
    _display.showClosingBarrier(_gate.getProgress());
    if (!_gate.isMoving() && _gate.isClosed()) {
        _buzzer.stop();
        _leds.setOrangeMode(LED_ON);
        transitionTo(SYS_ENTRY_INVITE_EXIT_USER);
    }
}

void SystemManager::handleEntryInviteExitUser(uint32_t now) {
    if (now == _stateEntry) {
        _display.showInviteExit();
        _buzzer.startContinuous();
    }

    if (stateElapsed(T_INVITE_LEAVE_ZONE_MS)) {
        _buzzer.stop();

        _leds.setPlaceState(_targetPlace, PLACE_OCCUPIED);
        _network.notifyPlaceOccupied(_targetPlace, _currentUid);

        transitionTo(SYS_ENTRY_REBALANCE);
    }
}

void SystemManager::handleEntryRebalance(uint32_t now) {
    if (now == _stateEntry) {
        int balancePlace = computeBalancedPlace();
        _display.showRotation();
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        _pivot.goToPlace(balancePlace);
    }

    if (_pivot.hasError()) {
        _buzzer.stop();
        enterErrorMode("PIVOT", "Rotation impossible");
        return;
    }

    if (!_pivot.isBusy() && _pivot.isAtTarget()) {
        _buzzer.stop();
        transitionTo(SYS_THANKS);
    }
}

// =============================================================================
//                       CYCLE SORTIE (Q8)
// =============================================================================

void SystemManager::handleExitRotateToPlace(uint32_t now) {
    if (now == _stateEntry) {
        _display.showRotation();
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        _leds.setBlueMode(LED_OFF);
        _pivot.goToPlace(_targetPlace);
    }

    if (_pivot.hasError()) {
        _buzzer.stop();
        enterErrorMode("PIVOT", "Rotation impossible");
        return;
    }

    if (!_pivot.isBusy() && _pivot.isAtTarget()) {
        _buzzer.stop();
        _leds.setOrangeMode(LED_ON);
        transitionTo(SYS_EXIT_ASK_ENTER_VEHICLE);
    }
}

void SystemManager::handleExitAskEnterVehicle(uint32_t now) {
    if (now == _stateEntry) {
        _display.showAskEnterVehicle();
        _leds.setOrangeMode(LED_ON);
    }
    if (stateElapsed(T_EXIT_BOARD_VEHICLE_MS)) {
        transitionTo(SYS_EXIT_ASK_WAIT);
    }
}

void SystemManager::handleExitAskWait(uint32_t now) {
    if (now == _stateEntry) {
        _display.showAskLowerVehicle();
    }
    if (stateElapsed(T_EXIT_WAIT_BARRIER_MS)) {
        _gate.openGate();
        _display.showOpeningBarrier(0);
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        transitionTo(SYS_EXIT_GATE_OPEN);
    }
}

void SystemManager::handleExitGateOpen(uint32_t now) {
    _display.showOpeningBarrier(_gate.getProgress());
    if (!_gate.isMoving() && _gate.isOpen()) {
        _buzzer.stop();
        _leds.setOrangeMode(LED_ON);
        transitionTo(SYS_EXIT_INVITE_LEAVE);
    }
}

void SystemManager::handleExitInviteLeave(uint32_t now) {
    if (now == _stateEntry) {
        _display.showInviteExit();
    }
    // Timer fixe car IR sortie HS (Q6)
    if (stateElapsed(T_EXIT_LEAVE_ZONE_MS)) {
        _gate.closeGate();
        _display.showClosingBarrier(0);
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        transitionTo(SYS_EXIT_GATE_CLOSE);
    }
}

void SystemManager::handleExitGateClose(uint32_t now) {
    _display.showClosingBarrier(_gate.getProgress());
    if (!_gate.isMoving() && _gate.isClosed()) {
        _buzzer.stop();

        _leds.setPlaceState(_targetPlace, PLACE_FREE);
        _network.notifyPlaceFreed(_targetPlace, _currentUid);

        transitionTo(SYS_EXIT_REBALANCE);
    }
}

void SystemManager::handleExitRebalance(uint32_t now) {
    if (now == _stateEntry) {
        int balancePlace = computeBalancedPlace();
        _display.showRotation();
        _buzzer.startSync(T_LED_BLINK_NORMAL_MS);
        _leds.setOrangeMode(LED_EXTERN_SYNC);
        _pivot.goToPlace(balancePlace);
    }

    if (_pivot.hasError()) {
        _buzzer.stop();
        enterErrorMode("PIVOT", "Rotation impossible");
        return;
    }

    if (!_pivot.isBusy() && _pivot.isAtTarget()) {
        _buzzer.stop();
        transitionTo(SYS_THANKS);
    }
}

// =============================================================================
//                       THANKS + ERROR
// =============================================================================

void SystemManager::handleThanks(uint32_t now) {
    if (now == _stateEntry) {
        _display.showThankYou();
        _buzzer.playHappy();
        _leds.setBlueMode(LED_BLINK_FAST);
        _leds.setOrangeMode(LED_OFF);
    }
    if (stateElapsed(T_THANKS_DISPLAY_MS)) {
        enterIdleMode();
    }
}

void SystemManager::enterErrorMode(const char* title, const char* msg) {
    _display.showError(title, msg);
    _buzzer.playError();
    _leds.setBlueMode(LED_OFF);
    _leds.setOrangeMode(LED_BLINK_FAST);
#if LOG_LEVEL >= 1
    Serial.print(F("[SYS] ERREUR: "));
    Serial.print(title);
    Serial.print(F(" - "));
    Serial.println(msg);
#endif
    transitionTo(SYS_ERROR);
}

void SystemManager::handleError(uint32_t now) {
    if (stateElapsed(T_ERROR_DISPLAY_MS)) {
        _pivot.resetError();
        enterIdleMode();
    }
}

// =============================================================================
//                       EQUILIBRAGE CARROUSEL (local)
// =============================================================================
// Logique locale strictement basee sur l'etat des places (LedsManager) :
//  - Aucune place occupee ou toutes occupees   -> position centrale (place 3)
//  - Une seule place occupee                   -> on l'amene en position basse
//  - Plusieurs places occupees                 -> on choisit la place mediane
// PivotServoManager garantit le chemin le plus court < 360 deg.
// =============================================================================

int SystemManager::computeBalancedPlace() const {
    int occupied[NUM_PLACES];
    int nOcc = 0;
    for (int i = 0; i < NUM_PLACES; i++) {
        if (_leds.getPlaceState(i) == PLACE_OCCUPIED) {
            occupied[nOcc++] = i;
        }
    }

    if (nOcc == 0 || nOcc == NUM_PLACES) {
        return PIVOT_REFERENCE_PLACE;
    }
    if (nOcc == 1) {
        return occupied[0];
    }
    return occupied[nOcc / 2];
}
