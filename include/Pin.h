#ifndef PIN_H
#define PIN_H

#include <Adafruit_SSD1306.h>

// IR sensors
#define TAPESENSOR_LEFT PA_0
#define TAPESENSOR_RIGHT PA_1
#define TAPESENSOR_RV PA_2

// Potentiometer inputs
#define KP_ADJUSTOR PA_3
#define KI_ADJUSTOR PA_4
#define KD_ADJUSTOR PA_5
#define TAPE_MIN_ADJUSTOR PA_6

// motors
#define MOTOR_L PA_8
#define MOTOR_R PA_9

// Flippy shenanigans
#define FLAPPER PA_10

// Dropoff shenanigans
#define SERVO PB1

// comparators (for checking if rv sensors are within a certain range)
#define RV_COMPARATOR PB10

#endif