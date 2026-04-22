#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <config.h>

class DisplayManager {
private:
    // Utilisation du constructeur SPI matériel validé
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;

    // Outils internes
    int getCenteredX(const char* text);
    int splitLines(const char* text, const char* lines[], int maxLines);
    void oledPrintTitle(const char* text);
    void oledPrintTextLines(const char* text, int bigLineIndex = -1);
    void oledDrawZoneFrames();
    void oledProgress(const char* title, const char* text, int progress);

public:
    DisplayManager();
    void begin();
    
    // Fonctions de contrôle
    void oledClear();
    void oledUpdate();

    // Méthodes d'affichage demandées
    void oledBootAnimation();
    void oledIdleScreen(int places = 7);
    void oledNewsScreen(const char* heure, const char* date, const char* temp, const char* hum);
    void oledOpeningBarrier(int progress);
    void oledClosingBarrier(int progress);
    void oledInviteEnter();
    void oledInviteAdvanceParking();
    void oledInviteExitVehicle();
    void oledInviteExit();
    void oledDangerRotation();
    void oledAskEnterVehicle();
    void oledAskLowerVehicle();
    void oledThankYou();
    void oledAccessGranted();
    void oledAccessDenied();
    void oledShowWebsite();
    void oledAdminWaitCard();
    void oledAdminResetOK();
};