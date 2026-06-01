#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

// =============================================================================
//                  PIVOT SERVO MANAGER (FB5311M-360)
// =============================================================================
// Servo a rotation continue + feedback analogique sur PIVOT_FB_PIN (0..4095).
//
// Principes :
//  - Le feedback FB renvoie l'angle physique courant 0..360 (cyclique).
//  - Un compteur de tours interne (_turnCount) absorbe les sauts FB 4095<->0
//    permettant un suivi lineaire continu (position absolue).
//  - Toute la logique haut-niveau raisonne en degres 0..360 (espace logique).
//  - L'offset Hall (place 3) est applique pour passer du raw FB a l'angle logique.
//
// Regles strictes (cahier des charges) :
//  - JAMAIS de tour complet (chemin angulaire < 360 deg toujours).
//  - Toujours emprunter le chemin le plus court (delta signe le plus petit).
//  - Securite : abort si la rotation depasse 360 deg sans atteindre la cible.
//  - Architecture 100% non-bloquante via update(now).
// =============================================================================

// Etats internes de la FSM pivot
enum PivotState {
    PIVOT_S_UNINIT,         // Avant begin()
    PIVOT_S_IDLE,           // Repos, servo detache
    PIVOT_S_CALIBRATING,    // Recherche Hall (rotation lente CW)
    PIVOT_S_MOVING,         // En route vers cible
    PIVOT_S_STOPPING,       // Stop demande, attente arret + delay avant detach
    PIVOT_S_ERROR           // Timeout ou anomalie de securite
};

class PivotServoManager {
public:
    PivotServoManager();

    // ---- Cycle de vie ----
    void begin();
    void update(uint32_t now);

    // ---- Calibration (place 3 = reference Hall) ----
    // 1. Appeler startCalibration() au boot.
    // 2. SystemManager observe le capteur Hall via SensorManager.
    //    Des qu'il detecte Hall, il appelle notifyHallTrigger() ici.
    // 3. Si Hall deja actif des le boot : appeler directement notifyHallTrigger()
    //    sans startCalibration() prealable (bypass rotation).
    void startCalibration();
    void notifyHallTrigger();
    bool isCalibrated() const;

    // ---- Commandes de deplacement ----
    // Retourne true si la commande est acceptee. Faux si occupe / non calibre.
    bool goToPlace(uint8_t placeIndex);
    bool goToAngle(float targetDeg);
    void stopNow();
    void resetError();

    // ---- Etat ----
    bool isBusy() const;
    bool isAtTarget() const;
    bool hasError() const;
    PivotState getState() const;

    // ---- Lectures ----
    float getCurrentAngle() const;          // Angle logique 0..360
    int   getCurrentPlace() const;          // 0..NUM_PLACES-1 (place la plus proche)
    int   getTargetPlace() const;           // -1 si pas de cible courante
    float getRotationProgress() const;      // 0..1 progression du mouvement

private:
    Servo _servo;
    bool _attached;
    PivotState _state;
    bool _calibrated;

    // Suivi feedback brut
    int _lastFb;                            // Derniere lecture FB (0..4095)
    int _turnCount;                         // Compteur de tours (sauts 4095<->0)
    float _currentAngleRaw;                 // Angle brut FB 0..360
    float _pivotOffset;                     // Offset logique = PIVOT_POS_3_DEG - angleRawAtHall

    // Cible courante
    float _targetAngleRaw;                  // Angle brut FB de la cible 0..360
    int _targetPlace;                       // -1 si goToAngle direct
    bool _targetReached;

    // Suivi lineaire du mouvement (en degres absolus = turnCount*360 + angleRaw)
    float _moveStartAbs;
    float _moveTargetAbs;
    float _currentAbs;

    // Timers
    uint32_t _lastFbPoll;
    uint32_t _moveStartTime;
    uint32_t _stopRequestTime;

    // ---- Helpers internes ----
    void  attachServo();
    void  detachServo();
    int   readFbRaw();
    void  updateFeedback();
    void  writeSpeed(int speed);
    void  writeStop();

    float placeToAngle(uint8_t placeIndex) const;
    static float wrap360(float a);
    static float shortestDelta(float from, float to);  // Resultat dans (-180..+180]
    static float fbToDeg(int fb);
    static int   degToFb(float deg);
};
