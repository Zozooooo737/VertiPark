#pragma once
#include <Arduino.h>
#include "config.h"

class LedsManager {
private:
    // Tableau représentant l'état de chaque place : true = occupée, false = libre
    bool _placesStatus[NUM_PLACES]; 

    // Envoie l'état actuel des places au registre à décalage
    void updateShiftRegister();

public:
    LedsManager();
    void begin();

    // Fonctions de contrôle
    void setPlaceStatus(int placeIndex, bool isOccupied);
    void setAllPlacesFree();
    void setAllPlacesOccupied();
    
    // Accesseur
    bool isPlaceOccupied(int placeIndex) const;
    
    void setBlueLed(bool turnOn);
    void setOrangeLed(bool turnOn);
};