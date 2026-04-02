#pragma once

// Capteurs IR
#define IR_SENSOR_ENTRY_PIN 34
#define IR_SENSOR_EXIT_PIN  35

#define IR_THRESHOLD 2000

// Servomoteur Barrière
#define GATE_SERVO_PIN 18

#define GATE_CLOSED_ANGLE 0
#define GATE_OPEN_ANGLE   90


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