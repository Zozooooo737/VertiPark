#pragma once

// Module RFID
#define PIN_RFID_RST      25
#define PIN_SPI_MISO      12
#define PIN_SPI_MOSI      13
#define PIN_SPI_SCK       14
#define PIN_RFID_CS       5

#define IR_THRESHOLD 2000

// Servomoteur Barrière
#define GATE_SERVO_PIN 18

#define GATE_CLOSED_ANGLE 0
#define GATE_OPEN_ANGLE   90

// Servo Pivot
#define PIVOT_SERVO_PIN 19
#define PIVOT_FB_PIN    34

#define PIVOT_STOP      90
#define PIVOT_CW        120 
#define PIVOT_CCW       60

// Afficheur OLED (SPI Hardware partagé)
#define OLED_CS    15
#define OLED_DC    26
#define OLED_RES   27
#define OLED_SCK   14
#define OLED_MOSI  13

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BLUE_ZONE_START 16


// Multiplexeur
#define MUX_SIG_PIN  36
#define MUX_SEL0_PIN 4
#define MUX_SEL1_PIN 2
#define MUX_SEL2_PIN 32

#define MUX_IR_ENTRY_CHANNEL    0
#define MUX_IR_EXIT_CHANNEL     1
#define MUX_AIR_QUALITY_CHANNEL 2
#define MUX_HALL_CHANNEL        3
#define MUX_LIGHT1_CHANNEL      4
#define MUX_LIGHT2_CHANNEL      5