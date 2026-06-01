#include "DisplayManager.h"

// =============================================================================
//             DISPLAY MANAGER - IMPLEMENTATION (non-bloquant)
// =============================================================================

DisplayManager::DisplayManager()
    : u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RES),
      _mode(SCREEN_NONE),
      _animDone(false),
      _animStart(0),
      _lastFrame(0),
      _animStep(0),
      _idlePlacesLibres(NUM_PLACES),
      _idleTemp(-999.0f),
      _idleHumidity(-999.0f),
      _idlePage(IDLE_PAGE_PLACES),
      _idlePageStart(0) {}

// -----------------------------------------------------------------------------
void DisplayManager::begin() {
    pinMode(OLED_CS, OUTPUT);
    pinMode(OLED_DC, OUTPUT);
    pinMode(OLED_RES, OUTPUT);

    // Reset hardware (uniquement appele dans setup)
    digitalWrite(OLED_RES, LOW);
    delay(100);
    digitalWrite(OLED_RES, HIGH);
    delay(100);

    u8g2.begin();
    oledClear();
    oledUpdate();

#if LOG_LEVEL >= 1
    Serial.println(F("[OLED] Initialise."));
#endif
}

// =============================================================================
//                            UPDATE FSM ANIMATIONS
// =============================================================================
void DisplayManager::update(uint32_t now) {
    switch (_mode) {
        case SCREEN_NONE:
        case SCREEN_STATIC:
            break;

        case SCREEN_BOOT_ANIM:
            if (now - _lastFrame >= T_BOOT_ANIM_STEP_MS) {
                _lastFrame = now;
                renderBootFrame();
                _animStep += 8;
                if (_animStep > 128) {
                    _animDone = true;
                    _mode = SCREEN_STATIC;
                }
            }
            break;

        case SCREEN_CALIBRATION:
            if (now - _lastFrame >= 400) {
                _lastFrame = now;
                _animStep = (_animStep + 1) % 4;
                renderCalibrationFrame();
            }
            break;

        case SCREEN_READY_ANIM:
            if (now - _lastFrame >= 200) {
                _lastFrame = now;
                _animStep++;
                renderReadyFrame();
                if (_animStep >= 6) {
                    _animDone = true;
                    _mode = SCREEN_STATIC;
                }
            }
            break;

        case SCREEN_IDLE_CYCLE:
            renderIdleCycle(now);
            break;
    }
}

// =============================================================================
//                            CONTROLE MODE
// =============================================================================

void DisplayManager::setMode(ScreenMode mode) {
    _mode = mode;
    _animStart = millis();
    _lastFrame = 0;
    _animStep = 0;
    _animDone = false;
    if (mode == SCREEN_IDLE_CYCLE) {
        _idlePage = IDLE_PAGE_PLACES;
        _idlePageStart = millis();
        renderIdlePage();
    }
}

ScreenMode DisplayManager::getMode() const { return _mode; }
bool DisplayManager::isAnimationDone() const { return _animDone; }

void DisplayManager::setIdleData(int placesLibres, float temp, float humidity) {
    _idlePlacesLibres = placesLibres;
    _idleTemp = temp;
    _idleHumidity = humidity;
}

// =============================================================================
//                            ANIMATIONS
// =============================================================================

void DisplayManager::startBootAnimation() {
    setMode(SCREEN_BOOT_ANIM);
    _animStep = 0;
}

void DisplayManager::startCalibrationAnimation() {
    setMode(SCREEN_CALIBRATION);
    renderCalibrationFrame();
}

void DisplayManager::startReadyAnimation() {
    setMode(SCREEN_READY_ANIM);
    renderReadyFrame();
}

// =============================================================================
//                            RENDU ANIMATIONS
// =============================================================================

void DisplayManager::renderBootFrame() {
    oledClear();
    oledPrintTitle("VERTIPARK");
    u8g2.drawFrame(0, 42, 128, 10);
    u8g2.drawBox(0, 42, _animStep, 10);
    oledUpdate();
}

void DisplayManager::renderCalibrationFrame() {
    oledClear();
    oledPrintTitle("CALIBRATION");
    char dots[8] = { 0 };
    int n = (_animStep % 4) + 1;
    for (int i = 0; i < n; i++) dots[i] = '.';
    oledPrintTextLines(dots, -1);
    oledUpdate();
}

void DisplayManager::renderReadyFrame() {
    oledClear();
    oledPrintTitle("VERTIPARK");
    if (_animStep % 2 == 0) {
        oledPrintTextLines("Systeme PRET", 0);
    }
    oledUpdate();
}

// =============================================================================
//                            IDLE CYCLE
// =============================================================================

void DisplayManager::renderIdleCycle(uint32_t now) {
    if (now - _idlePageStart >= T_IDLE_SCREEN_CYCLE_MS) {
        _idlePageStart = now;
        switch (_idlePage) {
            case IDLE_PAGE_PLACES:  _idlePage = IDLE_PAGE_NEWS;    break;
            case IDLE_PAGE_NEWS:    _idlePage = IDLE_PAGE_WEBSITE; break;
            case IDLE_PAGE_WEBSITE: _idlePage = IDLE_PAGE_PLACES;  break;
        }
        renderIdlePage();
    }
}

void DisplayManager::renderIdlePage() {
    switch (_idlePage) {
        case IDLE_PAGE_PLACES:  showIdle(_idlePlacesLibres);  break;
        case IDLE_PAGE_NEWS:    renderNewsPage();             break;
        case IDLE_PAGE_WEBSITE: renderWebsitePage();          break;
    }
    // showIdle() repasse en SCREEN_STATIC, on retablit le cycle
    _mode = SCREEN_IDLE_CYCLE;
}

void DisplayManager::renderNewsPage() {
    char buf[64];
    if (_idleTemp < -990.0f || _idleHumidity < -990.0f) {
        snprintf(buf, sizeof(buf), "Capteur DHT\nen lecture...");
    } else {
        snprintf(buf, sizeof(buf), "Temp %.1fC\nHum  %.0f%%",
                 _idleTemp, _idleHumidity);
    }
    oledClear();
    oledPrintTitle("AMBIANCE");
    oledPrintTextLines(buf, -1);
    oledUpdate();
}

void DisplayManager::renderWebsitePage() {
    oledClear();
    oledPrintTitle("INFO");
    oledPrintTextLines("Reservez sur\nvertipark.fr", -1);
    oledUpdate();
}

// =============================================================================
//                            ECRANS STATIQUES
// =============================================================================

void DisplayManager::showIdle(int placesLibres) {
    char text[40];
    snprintf(text, sizeof(text), "Places libres\n%d / %d", placesLibres, NUM_PLACES);
    oledClear();
    oledPrintTitle("VERTIPARK");
    oledPrintTextLines(text, 1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showNews(const char* h, const char* d, const char* t, const char* hu) {
    char text[64];
    snprintf(text, sizeof(text), "%s\n%s\n%s   %s", h, d, t, hu);
    oledClear();
    oledPrintTitle("VERTIPARK");
    oledPrintTextLines(text, 1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showWebsite() {
    renderWebsitePage();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showOpeningBarrier(int progress) {
    oledProgress("OUVERTURE", "Barriere...", progress);
    _mode = SCREEN_STATIC;
}

void DisplayManager::showClosingBarrier(int progress) {
    oledProgress("FERMETURE", "Barriere...", progress);
    _mode = SCREEN_STATIC;
}

void DisplayManager::showRotation(int progress) {
    if (progress < 0) {
        oledClear();
        oledPrintTitle("DANGER");
        oledPrintTextLines("Rotation\nen cours", -1);
        oledUpdate();
    } else {
        oledProgress("DANGER", "Rotation...", progress);
    }
    _mode = SCREEN_STATIC;
}

void DisplayManager::showInviteEnter() {
    oledClear();
    oledDrawZoneFrames();
    oledPrintTitle("OUVERT");
    oledPrintTextLines("Avancez jusqu'a\nla barriere", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showInviteAdvanceParking() {
    oledClear();
    oledPrintTitle("OUVERT");
    oledPrintTextLines("Avancez jusqu'a\nvotre place", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showInviteExitVehicle() {
    oledClear();
    oledPrintTitle("SORTIE");
    oledPrintTextLines("Descendez du\nvehicule", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showInviteExit() {
    oledClear();
    oledDrawZoneFrames();
    oledPrintTitle("ATTENTION");
    oledPrintTextLines("Quittez la zone", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showAskEnterVehicle() {
    oledClear();
    oledDrawZoneFrames();
    oledPrintTitle("ACTION");
    oledPrintTextLines("Montez dans\nvotre vehicule", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showAskLowerVehicle() {
    oledClear();
    oledDrawZoneFrames();
    oledPrintTitle("ATTENTION");
    oledPrintTextLines("Attendez ouverture\nbarriere", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showAccessGranted() {
    oledClear();
    oledPrintTitle("BIENVENUE");
    oledPrintTextLines("Acces autorise", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showAccessDenied(const char* reason) {
    oledClear();
    oledPrintTitle("ERREUR");
    if (reason && *reason) {
        char buf[40];
        snprintf(buf, sizeof(buf), "Acces refuse\n%s", reason);
        oledPrintTextLines(buf, -1);
    } else {
        oledPrintTextLines("Carte invalide", -1);
    }
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showThankYou() {
    oledClear();
    oledPrintTitle("MERCI");
    oledPrintTextLines("Bonne journee", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

void DisplayManager::showError(const char* title, const char* msg) {
    oledClear();
    oledPrintTitle(title ? title : "ERREUR");
    oledPrintTextLines(msg ? msg : "Anomalie systeme", -1);
    oledUpdate();
    _mode = SCREEN_STATIC;
}

// =============================================================================
//                            HELPERS DE RENDU
// =============================================================================

void DisplayManager::oledClear()  { u8g2.clearBuffer(); }
void DisplayManager::oledUpdate() { u8g2.sendBuffer(); }

int DisplayManager::getCenteredX(const char* text) {
    return (SCREEN_WIDTH - u8g2.getStrWidth(text)) / 2;
}

int DisplayManager::splitLines(const char* text, const char* lines[], int maxLines) {
    int count = 0;
    const char* start = text;
    const char* ptr = text;
    while (*ptr && count < maxLines) {
        if (*ptr == '\n') {
            static char buffers[10][64];
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
        if (len > 63) len = 63;
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

void DisplayManager::oledProgress(const char* title, const char* text, int progress) {
    if (progress < 0)   progress = 0;
    if (progress > 100) progress = 100;
    int barWidth = (progress * 108) / 100;

    oledClear();
    oledPrintTitle(title);
    oledPrintTextLines(text, -1);
    u8g2.drawFrame(10, 50, 108, 10);
    u8g2.drawBox(10, 50, barWidth, 10);
    oledUpdate();
}
