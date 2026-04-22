#include "DisplayManager.h"

// On utilise les noms définis dans config.h
DisplayManager::DisplayManager() : u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RES) {}

void DisplayManager::begin() {
    // On initialise le bus SPI avec les pins partagées
    // SCK=14, MISO=-1 (car pas besoin pour l'OLED), MOSI=13, CS=15
    
    digitalWrite(OLED_CS, LOW);
    u8g2.begin();
    digitalWrite(OLED_CS, HIGH);
    oledClear(); // Optionnel : démarre sur un écran propre
}

// --- OUTILS INTERNES ---

int DisplayManager::getCenteredX(const char* text) {
    return (SCREEN_WIDTH - u8g2.getStrWidth(text)) / 2;
}

int DisplayManager::splitLines(const char* text, const char* lines[], int maxLines) {
    int count = 0;
    const char* start = text;
    const char* ptr = text;
    while (*ptr && count < maxLines) {
        if (*ptr == '\n') {
            static char buffers[10][64]; // Pool de buffers pour éviter les problèmes de portée
            int len = ptr - start;
            if (len > 63) len = 63;
            strncpy(buffers[count], start, len);
            buffers[count][len] = '\0';
            lines[count] = buffers[count];
            count++;
            start = ptr + 1;
        }
        ptr++;
    }
    if (count < maxLines && start < ptr) {
        static char lastBuffer[64];
        int len = ptr - start;
        strncpy(lastBuffer, start, len);
        lastBuffer[len] = '\0';
        lines[count++] = lastBuffer;
    }
    return count;
}

void DisplayManager::oledPrintTitle(const char* text) {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    int x = getCenteredX(text);
    int y = u8g2.getAscent() + 1;
    u8g2.drawStr(x, y, text);
}

void DisplayManager::oledPrintTextLines(const char* text, int bigLineIndex) {
    const int maxLines = 10;
    const char* lines[maxLines];
    int nLines = splitLines(text, lines, maxLines);
    const int lineSpacing = 2;
    int totalHeight = 0;
    int lineHeights[maxLines];

    for (int i = 0; i < nLines; i++) {
        u8g2.setFont((i == bigLineIndex) ? u8g2_font_ncenB14_tr : u8g2_font_ncenB08_tr);
        lineHeights[i] = u8g2.getAscent() - u8g2.getDescent();
        totalHeight += lineHeights[i] + lineSpacing;
    }

    int y = BLUE_ZONE_START + ((SCREEN_HEIGHT - BLUE_ZONE_START) - totalHeight) / 2;
    for (int i = 0; i < nLines; i++) {
        u8g2.setFont((i == bigLineIndex) ? u8g2_font_ncenB14_tr : u8g2_font_ncenB08_tr);
        int x = getCenteredX(lines[i]);
        y += u8g2.getAscent();
        u8g2.drawStr(x, y, lines[i]);
        y += -u8g2.getDescent() + lineSpacing;
    }
}

void DisplayManager::oledDrawZoneFrames() {
    u8g2.drawFrame(0, BLUE_ZONE_START, SCREEN_WIDTH, SCREEN_HEIGHT - BLUE_ZONE_START);
}

void DisplayManager::oledClear() { u8g2.clearBuffer(); }
void DisplayManager::oledUpdate() { u8g2.sendBuffer(); }

// --- FONCTIONS D'AFFICHAGE ---

void DisplayManager::oledBootAnimation() {
    for (int i = 0; i <= 128; i += 4) {
        oledClear();
        oledPrintTitle("VERTIPARK");
        u8g2.drawFrame(0, 40, 128, 10);
        u8g2.drawBox(0, 40, i, 10);
        oledUpdate();
        delay(20);
    }
}

void DisplayManager::oledIdleScreen(int places) {
    char text[32];
    snprintf(text, sizeof(text), "Places restantes :\n%d", places);
    oledClear();
    oledPrintTitle("VERTIPARK");
    oledPrintTextLines(text, 1);
    oledUpdate();
}

void DisplayManager::oledNewsScreen(const char* h, const char* d, const char* t, const char* hu) {
    char text[64];
    snprintf(text, sizeof(text), "%s\n%s\n%s   %s", h, d, t, hu);
    oledClear();
    oledPrintTitle("VERTIPARK");
    oledPrintTextLines(text, 1);
    oledUpdate();
}

void DisplayManager::oledProgress(const char* title, const char* text, int progress) {
    oledClear();
    oledPrintTitle(title);
    oledPrintTextLines(text, -1);
    u8g2.drawFrame(10, 50, 108, 10);
    u8g2.drawBox(10, 50, progress, 10);
    oledUpdate();
}

void DisplayManager::oledOpeningBarrier(int p) { oledProgress("OUVERTURE", "Barriere...", p); }
void DisplayManager::oledClosingBarrier(int p) { oledProgress("FERMETURE", "Barriere...", p); }

void DisplayManager::oledInviteEnter() {
    oledClear(); oledDrawZoneFrames(); oledPrintTitle("OUVERT");
    oledPrintTextLines("Avancez jusqu'a\nla barriere", -1); oledUpdate();
}

void DisplayManager::oledInviteAdvanceParking() {
    oledClear(); oledPrintTitle("OUVERT");
    oledPrintTextLines("Avancez jusqu'a\nvotre place", -1); oledUpdate();
}

void DisplayManager::oledInviteExitVehicle() {
    oledClear(); oledPrintTitle("SORTIE");
    oledPrintTextLines("Merci de sortir\nde votre vehicule", -1); oledUpdate();
}

void DisplayManager::oledInviteExit() {
    oledClear(); oledDrawZoneFrames(); oledPrintTitle("ATTENTION");
    oledPrintTextLines("Quittez la zone", -1); oledUpdate();
}

void DisplayManager::oledDangerRotation() {
    oledClear(); oledPrintTitle("DANGER");
    oledPrintTextLines("Rotation en cours", -1); oledUpdate();
}

void DisplayManager::oledAskEnterVehicle() {
    oledClear(); oledDrawZoneFrames(); oledPrintTitle("ACTION");
    oledPrintTextLines("Montez dans\nvotre vehicule", -1); oledUpdate();
}

void DisplayManager::oledAskLowerVehicle() {
    oledClear(); oledDrawZoneFrames(); oledPrintTitle("ATTENTION");
    oledPrintTextLines("Descendez du\nvehicule", -1); oledUpdate();
}

void DisplayManager::oledThankYou() {
    oledClear(); oledPrintTitle("MERCI");
    oledPrintTextLines("Bonne journee", -1); oledUpdate();
}

void DisplayManager::oledAccessGranted() {
    oledClear(); oledPrintTitle("BIENVENUE");
    oledPrintTextLines("Acces autorise", -1); oledUpdate();
}

void DisplayManager::oledAccessDenied() {
    oledClear(); oledPrintTitle("ERREUR");
    oledPrintTextLines("Carte invalide", -1); oledUpdate();
}

void DisplayManager::oledShowWebsite() {
    oledClear(); oledPrintTitle("INFO");
    oledPrintTextLines("Plus d'infos ici :\nwww.vertipark.fr", -1); oledUpdate();
}

void DisplayManager::oledAdminWaitCard() {
    oledClear(); oledPrintTitle("ADMIN");
    oledPrintTextLines("Presenter carte...", -1); oledUpdate();
}

void DisplayManager::oledAdminResetOK() {
    oledClear(); oledPrintTitle("ADMIN");
    oledPrintTextLines("Carte reset", -1); oledUpdate();
}