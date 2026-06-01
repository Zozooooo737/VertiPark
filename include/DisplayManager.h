#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include "config.h"

// =============================================================================
//                       DISPLAY MANAGER (non-bloquant)
// =============================================================================
// Ecran OLED SSD1306 128x64 (SPI hardware partage avec RFID).
//
// Tous les ecrans "statiques" sont rendus en un seul appel.
// Les ANIMATIONS (boot, calibration, alternance idle/news/website) sont
// gerees par une FSM interne avec update(now) pour rester non-bloquant.
//
// Modes ecran (haut niveau) :
//   - SCREEN_NONE        : aucun rendu (laisse l'etat precedent)
//   - SCREEN_BOOT_ANIM   : barre de progression animee
//   - SCREEN_CALIBRATION : "Calibration..." + spinner / dots
//   - SCREEN_READY_ANIM  : courte animation succes au demarrage
//   - SCREEN_IDLE_CYCLE  : alterne idle / news / website automatiquement
//   - SCREEN_STATIC      : un ecran fixe (defini via showXxx())
//
// Le SystemManager passe d'un mode a l'autre via setMode(...) ou via les
// helpers showXxx() qui mettent le mode en SCREEN_STATIC automatiquement.
// =============================================================================

enum ScreenMode {
    SCREEN_NONE,
    SCREEN_BOOT_ANIM,
    SCREEN_CALIBRATION,
    SCREEN_READY_ANIM,
    SCREEN_IDLE_CYCLE,
    SCREEN_STATIC
};

// Sous-pages de l'idle cycle
enum IdlePage {
    IDLE_PAGE_PLACES,
    IDLE_PAGE_NEWS,
    IDLE_PAGE_WEBSITE
};

class DisplayManager {
public:
    DisplayManager();

    void begin();
    void update(uint32_t now);

    // ---- Controle de mode ----
    void setMode(ScreenMode mode);
    ScreenMode getMode() const;

    // ---- Donnees pour l'idle cycle ----
    void setIdleData(int placesLibres, float temp, float humidity);

    // ---- Animations ----
    void startBootAnimation();
    void startCalibrationAnimation();
    void startReadyAnimation();
    bool isAnimationDone() const;

    // ---- Ecrans statiques (passent automatiquement en SCREEN_STATIC) ----
    void showIdle(int placesLibres);
    void showNews(const char* heure, const char* date, const char* temp, const char* hum);
    void showWebsite();

    void showOpeningBarrier(int progress);
    void showClosingBarrier(int progress);
    void showRotation(int progress = -1);    // -1 = pas de barre

    void showInviteEnter();
    void showInviteAdvanceParking();
    void showInviteExitVehicle();
    void showInviteExit();
    void showAskEnterVehicle();
    void showAskLowerVehicle();

    void showAccessGranted();
    void showAccessDenied(const char* reason = nullptr);
    void showThankYou();
    void showError(const char* title, const char* msg);

    // ---- Acces bas niveau (compatibilite) ----
    void oledClear();
    void oledUpdate();

private:
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;

    ScreenMode _mode;
    bool _animDone;

    // Suivi animation
    uint32_t _animStart;
    uint32_t _lastFrame;
    int _animStep;

    // Donnees idle
    int _idlePlacesLibres;
    float _idleTemp;
    float _idleHumidity;
    IdlePage _idlePage;
    uint32_t _idlePageStart;

    // Helpers de rendu
    int  getCenteredX(const char* text);
    int  splitLines(const char* text, const char* lines[], int maxLines);
    void oledPrintTitle(const char* text);
    void oledPrintTextLines(const char* text, int bigLineIndex = -1);
    void oledDrawZoneFrames();
    void oledProgress(const char* title, const char* text, int progress);

    // Rendus animations
    void renderBootFrame();
    void renderCalibrationFrame();
    void renderReadyFrame();
    void renderIdleCycle(uint32_t now);
    void renderIdlePage();
    void renderNewsPage();
    void renderWebsitePage();
};
