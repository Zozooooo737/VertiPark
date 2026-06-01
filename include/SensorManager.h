#pragma once
#include <Arduino.h>
#include "config.h"

// =============================================================================
//                        SENSOR MANAGER (non-bloquant)
// =============================================================================
// Lit periodiquement (T_SENSOR_POLL_MS) les 4 canaux du multiplexeur :
//   - IR Entree
//   - IR Sortie (defectueux : valeur conservee mais non utilisee par FSM)
//   - Capteur Hall (position carrousel)
//   - Qualite air (MQ analogique)
//
// Toutes les valeurs sont mises en cache. Les getters retournent la derniere
// valeur valide sans relire le hardware (non bloquant).
//
// Anti-rebond logiciel par hysteresis temporelle (IR_DEBOUNCE_MS).
//
// Detection d'evenements front (rising / falling) consommable une seule fois :
//   - hasIrEntryRising()  / hasIrEntryFalling()
//   - hasHallRising()
// Ces flags sont mis a true au moment du front et remis a false par consume...().
// =============================================================================

class SensorManager {
public:
    SensorManager();

    void begin();
    void update(uint32_t now);

    // ---- Etats stabilises (apres debounce) ----
    bool isCarAtEntry() const;
    bool isCarAtExit()  const;   // Defectueux - laisse pour compat
    bool isHallTriggered() const;

    // ---- Valeurs analogiques brutes (dernier echantillon) ----
    int   getIrEntryRaw() const;
    int   getIrExitRaw()  const;
    int   getHallRaw()    const;
    float getAirQuality() const;

    // ---- Detection d'evenements (front) - consommables une seule fois ----
    bool consumeIrEntryRising();   // Vehicule arrive
    bool consumeIrEntryFalling();  // Vehicule s'est eloigne
    bool consumeHallRising();      // Aimant detecte

private:
    // Lecture brute hardware
    int readMuxChannel(uint8_t channel);

    // Etat brut (dernier echantillon)
    int _rawIrEntry;
    int _rawIrExit;
    int _rawHall;
    int _rawAir;

    // Etats stabilises (apres debounce)
    bool _stableIrEntry;
    bool _stableIrExit;
    bool _stableHall;

    // Etats candidats (apres seuillage, avant debounce)
    bool _candidateIrEntry;
    bool _candidateIrExit;
    bool _candidateHall;

    uint32_t _candidateIrEntryTime;
    uint32_t _candidateIrExitTime;
    uint32_t _candidateHallTime;

    // Flags d'evenements
    bool _evtIrEntryRising;
    bool _evtIrEntryFalling;
    bool _evtHallRising;

    // Timer global
    uint32_t _lastPoll;
};
