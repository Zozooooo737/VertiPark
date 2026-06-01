#pragma once

// =============================================================================
//                    VERTIPARK - CONFIGURATION GLOBALE
// =============================================================================
// Toutes les broches, timings, seuils et constantes systeme sont centralises ici.
// Le code applicatif ne doit JAMAIS contenir de valeurs magiques.
// =============================================================================

// ==========================================
// CONFIGURATION RÉSEAU (À modifier pour demain)
// ==========================================
const char* ssid = "iPhone de Mattia"; 
const char* password = "67676767";
// Remplace par l'IP de ton PC. Garde bien le /vertipark/api à la fin !
const String serverUrl = "http://172.20.10.2/vertipark/api";


// =============================================================================
//                              MODE DEBUG
// =============================================================================
// Activer pour utiliser HardwareTest dans main.cpp au lieu de la FSM normale.
// En production : laisser commente.
// -----------------------------------------------------------------------------
#define VERTIPARK_DEBUG_MODE

// Niveau de logs serie (0 = silencieux, 1 = essentiel, 2 = verbeux)
#define LOG_LEVEL 2


// =============================================================================
//                           MODULE RFID (MFRC522)
// =============================================================================
#define PIN_RFID_RST      25
#define PIN_SPI_MISO      12
#define PIN_SPI_MOSI      13
#define PIN_SPI_SCK       14
#define PIN_RFID_CS       5


// =============================================================================
//                       CAPTEURS IR & MULTIPLEXEUR
// =============================================================================
#define IR_THRESHOLD            2000   // Seuil ADC detection vehicule
#define IR_DEBOUNCE_MS          150    // Anti-rebond logiciel IR

// Multiplexeur 4 canaux
#define MUX_SIG_PIN             36
#define MUX_SEL0_PIN            4
#define MUX_SEL1_PIN            2

// Canaux du multiplexeur
#define MUX_IR_ENTRY_CHANNEL    0
#define MUX_IR_EXIT_CHANNEL     1   // Defectueux : remplace par timer (voir T_EXIT_*)
#define MUX_HALL_CHANNEL        2
#define MUX_AIR_QUALITY_CHANNEL 3


// =============================================================================
//                       SERVOMOTEUR BARRIERE
// =============================================================================
#define GATE_SERVO_PIN          18
#define GATE_CLOSED_ANGLE       30
#define GATE_OPEN_ANGLE         100
#define GATE_STEP_DEG           2      // Increment par tick (smooth)
#define GATE_STEP_INTERVAL_MS   15     // Delai entre 2 steps


// =============================================================================
//                       SERVO PIVOT (FB5311M-360)
// =============================================================================
// Servo rotation continue avec feedback analogique sur PIVOT_FB_PIN
// FB : 0..4095 ADC <=> 0..360 degres
// -----------------------------------------------------------------------------
#define PIVOT_SERVO_PIN         19
#define PIVOT_FB_PIN            34

#define PIVOT_STOP              90
#define PIVOT_CW                120
#define PIVOT_CCW               60
#define PIVOT_CW_SLOW           100    // Pour calibration douce
#define PIVOT_CCW_SLOW          80

// Seuils de detection de saut de tour (wrap FB 4095<->0)
#define PIVOT_FB_WRAP_HIGH      3500
#define PIVOT_FB_WRAP_LOW       500

// Tolerance d'arrivee sur une position cible (en unites FB, ~5 deg)
#define PIVOT_POS_TOLERANCE     60

// Periode de relecture du feedback pendant rotation (ms)
#define PIVOT_FB_POLL_MS        20

// Securite : duree maximale d'une rotation (ms) avant abort
#define PIVOT_ROTATION_TIMEOUT  20000

// Duree avant detach du servo apres stop (ms)
#define PIVOT_DETACH_DELAY_MS   200

// -----------------------------------------------------------------------------
// POSITIONS DES 7 PLACES (en degres 0..360)
// 7 places reparties uniformement, place 3 = reference centrale a 180 deg
// A CALIBRER PHYSIQUEMENT SUR INSTALLATION
// -----------------------------------------------------------------------------
#define PIVOT_POS_0_DEG   25.71f    // A CALIBRER PHYSIQUEMENT SUR INSTALLATION
#define PIVOT_POS_1_DEG   77.14f    // A CALIBRER PHYSIQUEMENT SUR INSTALLATION
#define PIVOT_POS_2_DEG   128.57f   // A CALIBRER PHYSIQUEMENT SUR INSTALLATION
#define PIVOT_POS_3_DEG   180.00f   // A CALIBRER PHYSIQUEMENT SUR INSTALLATION (reference Hall)
#define PIVOT_POS_4_DEG   231.43f   // A CALIBRER PHYSIQUEMENT SUR INSTALLATION
#define PIVOT_POS_5_DEG   282.86f   // A CALIBRER PHYSIQUEMENT SUR INSTALLATION
#define PIVOT_POS_6_DEG   334.29f   // A CALIBRER PHYSIQUEMENT SUR INSTALLATION

#define PIVOT_REFERENCE_PLACE   3   // Place alignee sur capteur Hall


// =============================================================================
//                       AFFICHEUR OLED (SPI partage)
// =============================================================================
#define OLED_CS                 15
#define OLED_DC                 26
#define OLED_RES                27
#define OLED_SCK                14
#define OLED_MOSI               13

#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           64
#define BLUE_ZONE_START         16


// =============================================================================
//                              BUZZER
// =============================================================================
#define BUZZER_PIN              33

// Frequences standard
#define BUZZ_FREQ_SUCCESS       2000
#define BUZZ_FREQ_INFO          1500
#define BUZZ_FREQ_ALERT         1200
#define BUZZ_FREQ_ERROR         500


// =============================================================================
//                              DHT22
// =============================================================================
#define DHT_PIN                 16
#define DHT_TYPE                DHT22


// =============================================================================
//                  LEDS PLACES (registre 74HC595) + AMBIANCE
// =============================================================================
#define NUM_PLACES              7

#define SHIFT_REG_DATA_PIN      21
#define SHIFT_REG_LATCH_PIN     22
#define SHIFT_REG_CLOCK_PIN     23

#define PIN_LED_BLUE            17
#define PIN_LED_ORANGE          32


// =============================================================================
//                  TIMINGS FSM (millis) - VERTIPARK
// =============================================================================
// Toutes les transitions non-bloquantes utilisent ces constantes.
// -----------------------------------------------------------------------------

// --- Boucle systeme ---
#define T_SENSOR_POLL_MS            50      // Polling capteurs (IR/Hall)
#define T_RFID_POLL_MS              200     // Polling RFID en IDLE
#define T_RFID_HALT_DELAY_MS        1500    // Anti-rebond meme badge

// --- Animations ecran (idle) ---
#define T_IDLE_SCREEN_CYCLE_MS      5000    // Alternance idle/news/website
#define T_DHT_REFRESH_MS            5000    // Lecture DHT22

// --- Boot & calibration ---
#define T_BOOT_ANIM_STEP_MS         30      // Step animation boot
#define T_CALIBRATION_TIMEOUT_MS    30000   // Securite calibration
#define T_READY_ANIM_MS             1500    // Anim "systeme pret"

// --- Cycle ENTREE ---
#define T_RFID_RESPONSE_TIMEOUT_MS  4000    // Timeout reponse serveur (futur)
#define T_ACCESS_GRANTED_DISPLAY_MS 2000    // Duree affichage "Bienvenue"
#define T_ACCESS_DENIED_DISPLAY_MS  2500    // Duree affichage refus
#define T_WAIT_CAR_AT_ENTRY_MS      15000   // Attente IR entree apres rotation
#define T_GATE_HOLD_OPEN_MS         10000   // Max barriere ouverte sans passage
#define T_AFTER_PASS_BEFORE_CLOSE_MS 3000   // Delai apres passage avant fermeture
#define T_INVITE_ADVANCE_MS         3000    // "Avancez jusqu'a votre place"
#define T_INVITE_EXIT_VEHICLE_MS    2000    // "Descendez du vehicule"
#define T_INVITE_LEAVE_ZONE_MS      2000    // "Quittez la zone" (buzzer continu)

// --- Cycle SORTIE ---
#define T_EXIT_BOARD_VEHICLE_MS     3000    // "Montez dans votre vehicule"
#define T_EXIT_WAIT_BARRIER_MS      1500    // "Attendez ouverture"
#define T_EXIT_LEAVE_ZONE_MS        5000    // Timer fermeture barriere sortie

// --- Thanks / Retour idle ---
#define T_THANKS_DISPLAY_MS         3000

// --- Error ---
#define T_ERROR_DISPLAY_MS          3000

// --- Buzzer patterns (non-bloquant) ---
#define T_BEEP_SHORT_MS             80
#define T_BEEP_LONG_MS              300
#define T_BEEP_INTER_MS             80

// --- Blink LED ambiance ---
#define T_LED_BLINK_FAST_MS         200
#define T_LED_BLINK_NORMAL_MS       500
#define T_LED_BLINK_SLOW_MS         800


// =============================================================================
//             CONFIGURATION RESEAU / API BACKEND (PHASE 2)
// =============================================================================
// Hooks prets a brancher. Le NetworkManager est stubbe en phase 1.
// -----------------------------------------------------------------------------
// VARIABLES A MODIFIER LORS DE L'ACTIVATION BACKEND :
// -----------------------------------------------------------------------------
#define WIFI_SSID               "VOTRE_SSID"          // A MODIFIER
#define WIFI_PASSWORD           "VOTRE_MOT_DE_PASSE"  // A MODIFIER

#define API_SERVER_HOST         "192.168.1.100"       // A MODIFIER (IP serveur PHP)
#define API_SERVER_PORT         80
#define API_BASE_PATH           "/vertipark/api"

// Endpoints serveur (cf. cahier des charges backend)
#define API_ENDPOINT_ACTION     "/action_parking.txt"  // GET   - commandes ENTREE:N / SORTIE:N
#define API_ENDPOINT_RFID       "/rfid_check.php"      // POST  - validation badge
#define API_ENDPOINT_STATUS     "/status.php"          // GET   - etat places
#define API_ENDPOINT_SENSOR     "/update_sensor.php"   // POST  - sync capteurs

// Polling backend
#define T_NETWORK_POLL_ACTION_MS    3000    // Lecture action_parking.txt
#define T_NETWORK_HTTP_TIMEOUT_MS   3000    // Timeout requete HTTP

// Activer pour reellement connecter le WiFi/HTTP (phase 2)
// #define VERTIPARK_NETWORK_ENABLED
