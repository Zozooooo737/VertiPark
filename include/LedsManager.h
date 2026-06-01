#pragma once
#include <Arduino.h>
#include "config.h"

// =============================================================================
//                          LEDS MANAGER
// =============================================================================
// Strictement visuel. Aucune logique metier / mecanique ici.
//
// Trois types de LEDs gerees :
//
//  1) LEDs des places (registre 74HC595)
//     - chaque place a une LED rouge/verte controlee par un bit
//     - 3 etats logiques : FREE / RESERVED / OCCUPIED
//        FREE      -> vert fixe
//        RESERVED  -> vert clignotant
//        OCCUPIED  -> rouge fixe
//
//  2) LED ambiance BLEUE (PIN_LED_BLUE)
//     - modes : OFF / ON / BLINK_FAST / BLINK_NORMAL / BLINK_SLOW
//
//  3) LED ambiance ORANGE (PIN_LED_ORANGE)
//     - modes : OFF / ON / BLINK_FAST / BLINK_NORMAL / BLINK_SLOW
//     - peut etre synchronisee avec un signal externe (ex: buzzer)
//
// Toutes les animations sont non-bloquantes via update(now).
// =============================================================================

enum PlaceState {
    PLACE_FREE      = 0,
    PLACE_RESERVED  = 1,
    PLACE_OCCUPIED  = 2
};

enum LedMode {
    LED_OFF,
    LED_ON,
    LED_BLINK_FAST,
    LED_BLINK_NORMAL,
    LED_BLINK_SLOW,
    LED_EXTERN_SYNC      // Pilote par toggle() externe (ex: buzzer sync tick)
};

class LedsManager {
public:
    LedsManager();

    void begin();
    void update(uint32_t now);

    // ---- Places ----
    void setPlaceState(uint8_t placeIndex, PlaceState state);
    void setAllPlacesFree();
    void setAllPlacesOccupied();
    PlaceState getPlaceState(uint8_t placeIndex) const;

    // ---- LED bleue ----
    void setBlueMode(LedMode mode);
    LedMode getBlueMode() const;

    // ---- LED orange ----
    void setOrangeMode(LedMode mode);
    LedMode getOrangeMode() const;

    // ---- Synchronisation externe ----
    // Lorsque la LED est en mode LED_EXTERN_SYNC, c'est un appel externe
    // a toggleOrange() / toggleBlue() qui fait clignoter.
    void toggleBlue();
    void toggleOrange();

    // ---- Tout eteindre ----
    void allOff();

private:
    PlaceState _places[NUM_PLACES];

    LedMode _blueMode;
    LedMode _orangeMode;

    bool _blueOn;
    bool _orangeOn;

    uint32_t _blueLastToggle;
    uint32_t _orangeLastToggle;

    // Etat clignotement places reservees
    bool _reservedBlinkOn;
    uint32_t _reservedLastToggle;

    void updateShiftRegister();
    void applyBlueLed();
    void applyOrangeLed();
    uint32_t periodForMode(LedMode mode) const;
};
